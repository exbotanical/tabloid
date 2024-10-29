#include "cursor.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "editor.h"
#include "globals.h"
#include "keypress.h"
#include "tty.h"

extern editor_t editor;

typedef enum {
  SELECT_LEFT,
  SELECT_LEFT_WORD,
  SELECT_RIGHT,
  SELECT_RIGHT_WORD
} select_mode_t;

static void
cursor_select (select_mode_t mode) {
  if (!editor.curs.select_active) {
    editor.curs.select_anchor.x = editor.curs.x;
    editor.curs.select_anchor.y = editor.curs.y;
  }
  editor.curs.select_active = true;
  switch (mode) {
    case SELECT_LEFT: cursor_move_left(); break;
    case SELECT_LEFT_WORD: cursor_move_left_word(); break;
    case SELECT_RIGHT: cursor_move_right(); break;
    case SELECT_RIGHT_WORD: cursor_move_right_word(); break;
  }

  editor.curs.select_offset.x = editor.curs.x;
  editor.curs.select_offset.y = editor.curs.y;
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
  return editor.curs.y == editor.r->num_lines - 1;
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
  return editor.curs.x >= editor.curs.col_off + (editor.win.cols - (line_pad + 1));
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
  int y     = editor.curs.y;
  int max_y = editor.r->num_lines - 1;
  if (y < max_y) {
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
    editor.curs.x
      = ((line_info_t *)array_get(editor.r->line_info, editor.curs.y))->line_length;
  }
}

void
cursor_move_right (void) {
  line_info_t *row
    = (editor.curs.y >= editor.r->num_lines)
        ? NULL
        : (line_info_t *)array_get(editor.r->line_info, editor.curs.y);
  if (row && editor.curs.x < row->line_length) {
    editor.curs.x++;
  } else if (row && editor.curs.x == row->line_length && !cursor_on_last_line()) {
    // Move to beginning of next line on right from last col
    editor.curs.y++;
    editor.curs.x = 0;
  }
}

void
cursor_move_right_word (void) {
  line_info_t *row
    = (editor.curs.y >= editor.r->num_lines)
        ? NULL
        : (line_info_t *)array_get(editor.r->line_info, editor.curs.y);

  if (editor.curs.x == row->line_length && !cursor_on_last_line()) {
    editor.curs.y++;
    editor.curs.x = 0;
    return;
  }

  // Jump to end if we're one char away
  if (editor.curs.x == row->line_length - 1) {
    editor.curs.x = row->line_length;
    return;
  }

  unsigned int i = editor.curs.x;

  // TODO: cache all lines in current window
  char buf[row->line_length];
  render_state_get_line(editor.r, editor.curs.y, buf);

  // If the very next char is a break char, jump to the start of the next word
  if (buf[i] == ' ') {
    for (; i < row->line_length; i++) {
      if (buf[i] != ' ') {
        break;
      }
    }
  } else {
    // Otherwise, if we're on a word, jump to the next break char
    for (; i < row->line_length; i++) {
      if (buf[i] == ' ') {
        break;
      }
    }
  }

  editor.curs.x = i;
}

void
cursor_move_left_word (void) {
  line_info_t *row
    = (editor.curs.y >= editor.r->num_lines)
        ? NULL
        : (line_info_t *)array_get(editor.r->line_info, editor.curs.y);

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

  char buf[row->line_length];
  render_state_get_line(editor.r, editor.curs.y, buf);

  // TODO: array or hashmap of break chars
  // If the preceding char is a break char, jump to the end of the next word
  if (buf[i - 1] == ' ') {
    for (; i > 0; i--) {
      if (buf[i - 1] != ' ') {
        break;
      }
    }
  } else {
    // Otherwise, if we're on a word, jump to the prev break char
    for (; i > 0; i--) {
      if (buf[i - 1] == ' ') {
        break;
      }
    }
  }

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
  editor.curs.y = (editor.curs.y > editor.r->num_lines)
                    ? editor.r->num_lines
                    : editor.curs.row_off + editor.win.rows - 1;
}

void
cursor_move_begin (void) {
  editor.curs.x = 0;
}

void
cursor_move_end (void) {
  if (editor.curs.y < editor.r->num_lines) {
    editor.curs.x
      = ((line_info_t *)array_get(editor.r->line_info, editor.curs.y))->line_length;
  }
}

// TODO: docs
// For moving up or down rows; makes sure the cursor jumps to the end if the line we moved from is longer.

// e.g.
// ----------------
// line 1
// this is line 2
// -----------------
// ^ cursor should snap to the end of line 1 when moving from the end of line 2 to line 1
void
cursor_snap_to_end (void) {
  line_info_t *row
    = (editor.curs.y >= editor.r->num_lines)
        ? NULL
        : (line_info_t *)array_get(editor.r->line_info, editor.curs.y);

  unsigned int length = row ? row->line_length : 0;
  if (editor.curs.x > length) {
    editor.curs.x = length;
  }
}

int
cursor_get_position (unsigned int *rows, unsigned int *cols) {
  char         buf[32];
  unsigned int i = 0;

  if (write(STDOUT_FILENO, ESC_SEQ "[6n", 4) != 4) {
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
    ESC_SEQ_CURSOR_POS_FMT,
    (editor.curs.y - editor.curs.row_off) + 1,
    ((editor.curs.x + line_pad + 1) - editor.curs.col_off) + 1
  );

  // If blinking block:
  // buffer_append(buf, "\x1b[2 q");
  buffer_append(buf, curs);
}

// TODO: no more global state
void
cursor_select_left (void) {
  cursor_select(SELECT_LEFT);
}

void
cursor_select_left_word (void) {
  cursor_select(SELECT_LEFT_WORD);
}

void
cursor_select_right (void) {
  cursor_select(SELECT_RIGHT);
}

void
cursor_select_right_word (void) {
  cursor_select(SELECT_RIGHT_WORD);
}

void
cursor_select_up (void) {
  if (!editor.curs.select_active) {
    editor.curs.select_anchor.x = editor.curs.x;
    editor.curs.select_anchor.y = editor.curs.y;
  }
  editor.curs.select_active = true;
  if (editor.curs.y == 0) {
    editor.curs.select_offset.x = 0;
    editor.curs.select_offset.y = 0;
    editor.curs.x               = 0;
    return;
  }

  cursor_move_up();
  editor.curs.select_offset.x = editor.curs.x;
  editor.curs.select_offset.y = editor.curs.y;
}

// TODO:
void
cursor_select_down (void) {
  if (!editor.curs.select_active) {
    editor.curs.select_anchor.x = editor.curs.x;
    editor.curs.select_anchor.y = editor.curs.y;
  }
  editor.curs.select_active = true;
  if (editor.curs.y == editor.r->num_lines - 1) {
    // TODO: err
    editor.curs.select_offset.x
      = ((line_info_t *)array_get(editor.r->line_info, editor.curs.y))->line_length;
    editor.curs.select_offset.y = editor.curs.y;
    editor.curs.x               = editor.curs.select_offset.x;
    return;
  }

  cursor_move_down();
  editor.curs.select_offset.x = editor.curs.x;
  editor.curs.select_offset.y = editor.curs.y;
}

void
cursor_select_clear (void) {
  editor.curs.select_active   = false;
  editor.curs.select_offset.x = -1;
  editor.curs.select_offset.y = -1;
  editor.curs.select_anchor.x = -1;
  editor.curs.select_anchor.y = -1;
}

bool
cursor_is_select_ltr (void) {
  return (
      editor.curs.select_anchor.y < editor.curs.select_offset.y ||
      (editor.curs.select_anchor.y == editor.curs.select_offset.y &&
        editor.curs.select_anchor.x <= editor.curs.select_offset.x)
    );
}
