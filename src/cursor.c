#include "cursor.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "editor.h"
#include "globals.h"
#include "keypress.h"
#include "tty.h"

extern editor_t editor;

bool
cursor_on_content_line (void) {
  return editor.curs.y < editor.buf.num_lines;
}

bool
cursor_on_first_line (void) {
  return editor.curs.y == 0;
}

bool
cursor_on_first_col (void) {
  return editor.curs.x == 0;
}

bool
cursor_on_last_line (void) {
  return editor.curs.y == editor.buf.num_lines - 1;
}

bool
cursor_above_visible_window (void) {
  return editor.curs.y < editor.curs.row_off;
}

bool
cursor_below_visible_window (void) {
  return editor.curs.y >= editor.curs.row_off + editor.win.rows;
}

bool
cursor_left_of_visible_window (void) {
  return editor.curs.x < editor.curs.col_off;
}

bool
cursor_right_of_visible_window (void) {
  return editor.curs.x >= editor.curs.col_off + editor.win.cols;
}

bool
cursor_in_cell_zero (void) {
  return editor.curs.x == 0 && editor.curs.y == 0;
}

bool
cursor_not_at_row_begin (void) {
  return editor.curs.x > 0;
}

void
cursor_move_down (void) {
  if (editor.curs.y < editor.buf.num_lines - 1) {
    editor.curs.y++;
  }
}

void
cursor_move_up (void) {
  if (!cursor_on_first_line()) {
    editor.curs.y--;
  }
}

void
cursor_move_left (void) {
  if (!cursor_on_first_col()) {
    editor.curs.x--;
  } else if (!cursor_on_first_line()) {
    // Move to end of prev line on left from col 0
    editor.curs.y--;
    editor.curs.x = editor.buf.lines[editor.curs.y].render_buf_sz;
  }
}

void
cursor_move_right (void) {
  line_buffer_t *row = (editor.curs.y >= editor.buf.num_lines)
                         ? NULL
                         : &editor.buf.lines[editor.curs.y];
  if (row && editor.curs.x < row->render_buf_sz) {
    editor.curs.x++;
  } else if (row && editor.curs.x == row->render_buf_sz && !cursor_on_last_line()) {
    // Move to beginning of next line on right from last col
    editor.curs.y++;
    editor.curs.x = 0;
  }
}

void
cursor_move_right_word (void) {
  line_buffer_t *row = (editor.curs.y >= editor.buf.num_lines)
                         ? NULL
                         : &editor.buf.lines[editor.curs.y];

  if (editor.curs.x == row->render_buf_sz && !cursor_on_last_line()) {
    editor.curs.y++;
    editor.curs.x = 0;
    return;
  }

  // Jump to end if we're one char away
  if (editor.curs.x == row->render_buf_sz - 1) {
    editor.curs.x = row->render_buf_sz;
    return;
  }

  unsigned int i = editor.curs.x;

  // If the very next char is a break char, jump to the start of the next word
  if (row->render_buf[i] == ' ') {
    for (; i < row->render_buf_sz; i++) {
      if (row->render_buf[i] != ' ') {
        break;
      }
    }
  } else {
    // Otherwise, if we're on a word, jump to the next break char
    for (; i < row->render_buf_sz; i++) {
      if (row->render_buf[i] == ' ') {
        break;
      }
    }
  }

  editor.curs.x = i;
}

void
cursor_move_left_word (void) {
  line_buffer_t *row = (editor.curs.y >= editor.buf.num_lines)
                         ? NULL
                         : &editor.buf.lines[editor.curs.y];

  if (editor.curs.x == 0) {
    cursor_move_left();
    return;
  }

  // Jump to beginning if we're one char away
  if (editor.curs.x == 1) {
    editor.curs.x = 0;
    return;
  }

  unsigned int i = editor.curs.x;

  // TODO: array or hashmap of break chars
  // If the preceding char is a break char, jump to the end of the next word
  if (row->render_buf[i - 1] == ' ') {
    for (; i > 0; i--) {
      if (row->render_buf[i - 1] != ' ') {
        break;
      }
    }
  } else {
    // Otherwise, if we're on a word, jump to the prev break char
    for (; i > 0; i--) {
      if (row->render_buf[i - 1] == ' ') {
        break;
      }
    }
  }
  logger.write("i is %d\n", i);
  editor.curs.x = i;
}

void
cursor_move_top (void) {
  editor.curs.y = 0;
}

void
cursor_move_visible_top (void) {
  editor.curs.y = editor.curs.row_off;
}

void
cursor_move_bottom (void) {
  editor.curs.y = editor.win.rows - 1;
}

void
cursor_move_visible_bottom (void) {
  editor.curs.y = (editor.curs.y > editor.buf.num_lines)
                    ? editor.buf.num_lines
                    : editor.curs.row_off + editor.win.rows - 1;
}

void
cursor_move_begin (void) {
  editor.curs.x = 0;
}

void
cursor_move_end (void) {
  if (editor.curs.y < editor.buf.num_lines) {
    editor.curs.x = editor.buf.lines[editor.curs.y].render_buf_sz;
  }
}

void
cursor_snap_to_end (void) {
  line_buffer_t *row   = (editor.curs.y >= editor.buf.num_lines)
                           ? NULL
                           : &editor.buf.lines[editor.curs.y];

  unsigned int row_len = row ? row->render_buf_sz : 0;
  if (editor.curs.x > row_len) {
    editor.curs.x = row_len;
  }
}

int
cursor_get_position (unsigned int *rows, unsigned int *cols) {
  char         buf[32];
  unsigned int i = 0;

  if (write(STDOUT_FILENO, ESCAPE_SEQ "[6n", 4) != 4) {
    return -1;
  }

  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1) {
      break;
    }
    if (buf[i] == 'R') {
      break;
    }
    i++;
  }
  buf[i] = '\0';

  if (buf[0] != '\x1b' || buf[1] != '[') {
    return -1;
  }
  if (sscanf(&buf[2], "%u;%u", rows, cols) != 2) {
    return -1;
  }

  return 0;
}

void
cursor_set_position (buffer_t *buf) {
  char curs[32];

  snprintf(
    curs,
    sizeof(curs),
    ESCAPE_SEQ_CURSOR_POS_FMT,
    (editor.curs.y - editor.curs.row_off) + 1,
    (editor.curs.x - editor.curs.col_off) + 1
  );

  // If blinking block:
  // buffer_append(buf, "\x1b[2 q");
  buffer_append(buf, curs);
}
