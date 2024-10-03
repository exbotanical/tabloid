#include "cursor.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "editor.h"
#include "keypress.h"
#include "tty.h"

extern editor_t editor;

bool
cursor_on_content_line (void) {
  return editor.curs.y < editor.buf.num_rows;
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
cursor_above_visible_window (void) {
  return editor.curs.y < editor.curs.row;
}

bool
cursor_below_visible_window (void) {
  return editor.curs.y >= editor.curs.row + editor.win.rows;
}

bool
cursor_left_of_visible_window (void) {
  return editor.renderx < editor.curs.col;
}

bool
cursor_right_of_visible_window (void) {
  return editor.renderx >= editor.curs.col + editor.win.cols;
}

void
cursor_move_down (void) {
  if (editor.curs.y < editor.buf.num_rows) {
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
    editor.curs.x = editor.buf.rows[editor.curs.y].raw_sz;
  }
}

void
cursor_move_right (void) {
  row_buffer_t *row = (editor.curs.y >= editor.buf.num_rows)
                        ? NULL
                        : &editor.buf.rows[editor.curs.y];

  if (row && editor.curs.x < row->raw_sz) {
    editor.curs.x++;
  } else if (row && editor.curs.x == row->raw_sz) {
    // Move to beginning of next line on right from last col
    editor.curs.y++;
    editor.curs.x = 0;
  }
}

void
cursor_move_top (void) {
  editor.curs.y = 0;
}

void
cursor_move_visible_top (void) {
  editor.curs.y = editor.curs.row;
}

void
cursor_move_bottom (void) {
  editor.curs.y = editor.win.rows - 1;
}

void
cursor_move_visible_bottom (void) {
  editor.curs.y = (editor.curs.y > editor.buf.num_rows)
                    ? editor.buf.num_rows
                    : editor.curs.row + editor.win.rows - 1;
}

void
cursor_move_begin (void) {
  editor.curs.x = 0;
}

void
cursor_move_end (void) {
  if (editor.curs.y < editor.buf.num_rows) {
    editor.curs.x = editor.buf.rows[editor.curs.y].raw_sz;
  }
}

void
cursor_snap_to_end (void) {
  row_buffer_t *row    = (editor.curs.y >= editor.buf.num_rows)
                           ? NULL
                           : &editor.buf.rows[editor.curs.y];

  unsigned int row_len = row ? row->raw_sz : 0;
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
    (editor.curs.y - editor.curs.row) + 1,
    (editor.renderx - editor.curs.col) + 1
  );

  buffer_append(buf, curs);
}
