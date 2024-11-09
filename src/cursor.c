#include "cursor.h"

#include <assert.h>
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
  SELECT_RIGHT_WORD,
} select_mode_t;

static void
cursor_set_select_anchor (int x, int y) {
  editor.curs.select_anchor.x = x;
  editor.curs.select_anchor.y = y;
}

static void
cursor_set_select_offset (int x, int y) {
  editor.curs.select_offset.x = x;
  editor.curs.select_offset.y = y;
}

static void
cursor_copy_to_select_anchor (void) {
  cursor_set_select_anchor(cursor_get_x(), cursor_get_y());
}

static void
cursor_copy_to_select_offset (void) {
  cursor_set_select_offset(cursor_get_x(), cursor_get_y());
}

static void
cursor_select (select_mode_t mode) {
  if (!cursor_is_select_active()) {
    cursor_copy_to_select_anchor();
  }

  cursor_set_is_active(true);

  switch (mode) {
    case SELECT_LEFT: cursor_move_left(); break;
    case SELECT_LEFT_WORD: cursor_move_left_word(); break;
    case SELECT_RIGHT: cursor_move_right(); break;
    case SELECT_RIGHT_WORD: cursor_move_right_word(); break;
  }

  cursor_copy_to_select_offset();
}

inline unsigned int
cursor_get_x (void) {
  return editor.curs.x;
}

inline unsigned int
cursor_get_y (void) {
  return editor.curs.y;
}

inline unsigned int
cursor_get_anchor_x (void) {
  return editor.curs.select_anchor.x;
}

inline unsigned int
cursor_get_anchor_y (void) {
  return editor.curs.select_anchor.y;
}

inline unsigned int
cursor_get_offset_x (void) {
  return editor.curs.select_offset.x;
}

// TODO: PAY CC!!!!
inline unsigned int
cursor_get_offset_y (void) {
  return editor.curs.select_offset.y;
}

inline unsigned int
cursor_get_row_off (void) {
  return editor.curs.row_off;
}

inline unsigned int
cursor_get_col_off (void) {
  return editor.curs.col_off;
}

inline bool
cursor_is_select_active (void) {
  return editor.curs.select_active;
}

inline void
cursor_set_xy (unsigned int x, unsigned int y) {
  cursor_set_x(x);
  cursor_set_y(y);
}

inline void
cursor_set_x (unsigned int x) {
  editor.curs.x = x;
}

inline unsigned int
cursor_inc_x (void) {
  editor.curs.x++;
  return cursor_get_x();
}

inline unsigned int
cursor_dec_x (void) {
  editor.curs.x--;
  return cursor_get_x();
}

inline void
cursor_set_y (unsigned int y) {
  editor.curs.y = y;
}

inline unsigned int
cursor_inc_y (void) {
  editor.curs.y++;
  return cursor_get_y();
}

inline unsigned int
cursor_dec_y (void) {
  editor.curs.y--;
  return cursor_get_y();
}

inline void
cursor_set_row_off (unsigned int row_off) {
  editor.curs.row_off = row_off;
}

inline void
cursor_set_col_off (unsigned int col_off) {
  editor.curs.col_off = col_off;
}

inline void
cursor_set_is_active (bool next) {
  editor.curs.select_active = next;
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
  // clang-format off
  snprintf(
    curs,
    sizeof(curs),
    ESC_SEQ_CURSOR_POS_FMT,
    (cursor_get_y() - cursor_get_row_off()) + 1,
    ((cursor_get_x() + line_pad + 1) - cursor_get_col_off()) + 1
  );
  // clang-format on
  buffer_append(buf, curs);
}

cursor_t *
cursor_create_copy (void) {
  cursor_t *curs = malloc(sizeof(cursor_t));
  curs->x        = cursor_get_x();
  curs->y        = cursor_get_y();
  curs->col_off  = cursor_get_col_off();

  return curs;
}

void
cursor_move_down (void) {
  int max_y = editor.r->num_lines - 1;
  if ((int)cursor_get_y() < max_y) {
    cursor_inc_y();
  }
}

void
cursor_move_up (void) {
  if (!cursor_on_first_line()) {
    cursor_dec_y();
  }
}

void
cursor_move_left (void) {
  if (!cursor_on_first_col()) {
    cursor_dec_x();
  } else if (!cursor_on_first_line()) {
    // Move to end of prev line on left from col 0
    line_info_t *line_info = (line_info_t *)array_get(editor.r->line_info, cursor_dec_y());
    assert(line_info != NULL);
    cursor_set_x(line_info->line_length);
  }
}

void
cursor_move_left_word (void) {
  line_info_t *line_info = (line_info_t *)array_get(editor.r->line_info, cursor_get_y());
  assert(line_info != NULL);

  if (cursor_get_x() == 0) {
    cursor_move_left();
    return;
  }

  // Jump to beginning if we're one char away
  if (cursor_get_x() == 1) {
    cursor_set_x(0);
    return;
  }

  unsigned int i = cursor_get_x();

  char buf[line_info->line_length];
  line_buffer_get_line(editor.r, cursor_get_y(), buf);

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

  cursor_set_x(i);
}

void
cursor_move_right (void) {
  line_info_t *line_info = (line_info_t *)array_get(editor.r->line_info, cursor_get_y());
  if (!line_info) {
    return;
  }

  if (cursor_get_x() < line_info->line_length) {
    cursor_inc_x();
  } else if (cursor_get_x() == line_info->line_length && !cursor_on_last_line()) {
    // Move to beginning of next line on right from last col
    cursor_set_x(0);
    cursor_inc_y();
  }
}

void
cursor_move_right_word (void) {
  line_info_t *line_info = (line_info_t *)array_get(editor.r->line_info, cursor_get_y());
  assert(line_info != NULL);

  if (cursor_get_x() == line_info->line_length && !cursor_on_last_line()) {
    cursor_set_x(0);
    cursor_inc_y();
    return;
  }

  // Jump to end if we're one char away
  if (cursor_get_x() == line_info->line_length - 1) {
    cursor_set_x(line_info->line_length);
    return;
  }

  unsigned int i = cursor_get_x();

  // TODO: cache all lines in current window
  char buf[line_info->line_length];
  line_buffer_get_line(editor.r, cursor_get_y(), buf);

  // If the very next char is a break char, jump to the start of the next word
  if (buf[i] == ' ') {
    for (; i < line_info->line_length; i++) {
      if (buf[i] != ' ') {
        break;
      }
    }
  } else {
    // Otherwise, if we're on a word, jump to the next break char
    for (; i < line_info->line_length; i++) {
      if (buf[i] == ' ') {
        break;
      }
    }
  }

  cursor_set_x(i);
}

void
cursor_move_top (void) {
  cursor_set_y(0);
}

void
cursor_move_visible_top (void) {
  cursor_set_y(cursor_get_row_off());
}

void
cursor_move_bottom (void) {
  cursor_set_y(window_get_num_rows() - 1);
}

void
cursor_move_visible_bottom (void) {
  cursor_set_y((cursor_get_y() > editor.r->num_lines) ? editor.r->num_lines : cursor_get_row_off() + window_get_num_rows() - 1);
}

void
cursor_move_begin (void) {
  cursor_set_x(0);
}

void
cursor_move_end (void) {
  if (cursor_get_y() < editor.r->num_lines) {
    line_info_t *line_info = (line_info_t *)array_get(editor.r->line_info, cursor_get_y());
    assert(line_info != NULL);
    cursor_set_x(line_info->line_length);
  }
}

// TODO: docs
// For moving up or down rows; makes sure the cursor jumps to the end if the
// line we moved from is longer.

// e.g.
// ----------------
// line 1
// this is line 2
// -----------------
// ^ cursor should snap to the end of line 1 when moving from the end of line 2
// to line 1
void
cursor_snap_to_end (void) {
  line_info_t *line_info = (line_info_t *)array_get(editor.r->line_info, cursor_get_y());

  unsigned int length    = line_info ? line_info->line_length : 0;
  if (cursor_get_x() > length) {
    cursor_set_x(length);
  }
}

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
  if (!cursor_is_select_active()) {
    cursor_copy_to_select_anchor();
  }

  cursor_set_is_active(true);

  if (cursor_get_y() == 0) {
    cursor_set_select_offset(0, 0);
    cursor_set_x(0);
    return;
  }

  cursor_move_up();
  cursor_copy_to_select_offset();
}

void
cursor_select_down (void) {
  if (!cursor_is_select_active()) {
    cursor_copy_to_select_anchor();
  }

  cursor_set_is_active(true);

  if (cursor_get_y() == editor.r->num_lines - 1) {
    line_info_t *line_info = (line_info_t *)array_get(editor.r->line_info, cursor_get_y());
    assert(line_info != NULL);
    cursor_set_select_offset(line_info->line_length, cursor_get_y());
    cursor_set_x(editor.curs.select_offset.x);
    return;
  }

  cursor_move_down();
  cursor_copy_to_select_offset();
}

bool
cursor_is_select_ltr (void) {
  return (
      editor.curs.select_anchor.y < editor.curs.select_offset.y ||
      (editor.curs.select_anchor.y == editor.curs.select_offset.y &&
        editor.curs.select_anchor.x <= editor.curs.select_offset.x)
    );
}

void
cursor_select_clear (void) {
  cursor_set_is_active(false);
  cursor_set_select_offset(-1, -1);
  cursor_set_select_anchor(-1, -1);
}

bool
cursor_on_first_line (void) {
  return cursor_get_y() == 0;
}

bool
cursor_on_first_col (void) {
  return cursor_get_x() == 0;
}

bool
cursor_on_last_line (void) {
  return cursor_get_y() == editor.r->num_lines - 1;
}

bool
cursor_above_visible_window (void) {
  return cursor_get_y() < cursor_get_row_off();
}

bool
cursor_below_visible_window (void) {
  return cursor_get_y() >= cursor_get_row_off() + window_get_num_rows();
}

bool
cursor_left_of_visible_window (void) {
  return cursor_get_x() < cursor_get_col_off();
}

bool
cursor_right_of_visible_window (void) {
  return cursor_get_x() >= cursor_get_col_off() + (window_get_num_cols() - (line_pad + 1));
}

bool
cursor_in_cell_zero (void) {
  return cursor_get_x() == 0 && cursor_get_y() == 0;
}

bool
cursor_not_at_row_begin (void) {
  return cursor_get_x() > 0;
}
