#include "cursor.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "command_bar.h"
#include "globals.h"
#include "keypress.h"
#include "tty.h"

typedef enum {
  SELECT_LEFT,
  SELECT_LEFT_WORD,
  SELECT_RIGHT,
  SELECT_RIGHT_WORD,
} select_mode_t;

static void
cursor_set_select_anchor (line_editor_t *self, int x, int y) {
  self->curs.select_anchor.x = x;
  self->curs.select_anchor.y = y;
}

static void
cursor_set_select_offset (line_editor_t *self, int x, int y) {
  self->curs.select_offset.x = x;
  self->curs.select_offset.y = y;
}

static void
cursor_copy_to_select_anchor (line_editor_t *self) {
  cursor_set_select_anchor(self, cursor_get_x(self), cursor_get_y(self));
}

static void
cursor_copy_to_select_offset (line_editor_t *self) {
  cursor_set_select_offset(self, cursor_get_x(self), cursor_get_y(self));
}

static void
cursor_select (line_editor_t *self, select_mode_t mode) {
  if (!cursor_is_select_active(self)) {
    cursor_copy_to_select_anchor(self);
  }

  cursor_set_is_active(self, true);

  switch (mode) {
    case SELECT_LEFT: cursor_move_left(self); break;
    case SELECT_LEFT_WORD: cursor_move_left_word(self); break;
    case SELECT_RIGHT: cursor_move_right(self); break;
    case SELECT_RIGHT_WORD: cursor_move_right_word(self); break;
  }

  cursor_copy_to_select_offset(self);
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
cursor_set_position (line_editor_t *self, buffer_t *buf) {
  char curs[32];
  // clang-format off
  snprintf(
    curs,
    sizeof(curs),
    ESC_SEQ_CURSOR_POS_FMT,
    (cursor_get_y(self) - cursor_get_row_off(self)) + 1,
    ((cursor_get_x(self) + line_pad + 1) - cursor_get_col_off(self)) + 1
  );
  // clang-format on
  buffer_append(buf, curs);
}

void
cursor_set_position_command_bar (line_editor_t *self, buffer_t *buf) {
  char curs[32];
  // clang-format off
  snprintf(
    curs,
    sizeof(curs),
    ESC_SEQ_CURSOR_POS_FMT,
    window_get_num_rows() + 2,
    (cursor_get_x(self) - cursor_get_col_off(self)) + 1 + COMMAND_BAR_PREFIX_OFFSET
  );
  // clang-format on
  buffer_append(buf, curs);
}

cursor_t *
cursor_create_copy (line_editor_t *self) {
  cursor_t *curs = malloc(sizeof(cursor_t));
  curs->x        = cursor_get_x(self);
  curs->y        = cursor_get_y(self);
  curs->col_off  = cursor_get_col_off(self);

  return curs;
}

void
cursor_move_down (line_editor_t *self) {
  int max_y = self->r->num_lines - 1;
  if ((int)cursor_get_y(self) < max_y) {
    cursor_inc_y(self);
  }
}

void
cursor_move_up (line_editor_t *self) {
  if (!cursor_on_first_line(self)) {
    cursor_dec_y(self);
  }
}

void
cursor_move_left (line_editor_t *self) {
  if (!cursor_on_first_col(self)) {
    cursor_dec_x(self);
  } else if (!cursor_on_first_line(self)) {
    // Move to end of prev line on left from col 0
    line_info_t *line_info = (line_info_t *)array_get(self->r->line_info, cursor_dec_y(self));
    assert(line_info != NULL);
    cursor_set_x(self, line_info->line_length);
  }
}

void
cursor_move_left_word (line_editor_t *self) {
  line_info_t *line_info = (line_info_t *)array_get(self->r->line_info, cursor_get_y(self));
  assert(line_info != NULL);

  if (cursor_get_x(self) == 0) {
    cursor_move_left(self);
    return;
  }

  // Jump to beginning if we're one char away
  if (cursor_get_x(self) == 1) {
    cursor_set_x(self, 0);
    return;
  }

  unsigned int i = cursor_get_x(self);

  char buf[line_info->line_length];
  line_buffer_get_line(self->r, cursor_get_y(self), buf);

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

  cursor_set_x(self, i);
}

void
cursor_move_right (line_editor_t *self) {
  line_info_t *line_info = (line_info_t *)array_get(self->r->line_info, cursor_get_y(self));
  if (!line_info) {
    return;
  }

  if (cursor_get_x(self) < line_info->line_length) {
    cursor_inc_x(self);
  } else if (cursor_get_x(self) == line_info->line_length && !cursor_on_last_line(self)) {
    // Move to beginning of next line on right from last col
    cursor_set_x(self, 0);
    cursor_inc_y(self);
  }
}

void
cursor_move_right_word (line_editor_t *self) {
  line_info_t *line_info = (line_info_t *)array_get(self->r->line_info, cursor_get_y(self));
  assert(line_info != NULL);

  if (cursor_get_x(self) == line_info->line_length && !cursor_on_last_line(self)) {
    cursor_set_x(self, 0);
    cursor_inc_y(self);
    return;
  }

  // Jump to end if we're one char away
  if (cursor_get_x(self) == line_info->line_length - 1) {
    cursor_set_x(self, line_info->line_length);
    return;
  }

  unsigned int i = cursor_get_x(self);

  // TODO: cache all lines in current window
  char buf[line_info->line_length];
  line_buffer_get_line(self->r, cursor_get_y(self), buf);

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

  cursor_set_x(self, i);
}

void
cursor_move_top (line_editor_t *self) {
  cursor_set_y(self, 0);
}

void
cursor_move_visible_top (line_editor_t *self) {
  cursor_set_y(self, cursor_get_row_off(self));
}

void
cursor_move_bottom (line_editor_t *self) {
  cursor_set_y(self, window_get_num_rows() - 1);
}

void
cursor_move_visible_bottom (line_editor_t *self) {
  cursor_set_y(self, (cursor_get_y(self) > self->r->num_lines) ? self->r->num_lines : cursor_get_row_off(self) + window_get_num_rows() - 1);
}

void
cursor_move_begin (line_editor_t *self) {
  cursor_set_x(self, 0);
}

void
cursor_move_end (line_editor_t *self) {
  if (cursor_get_y(self) < self->r->num_lines) {
    line_info_t *line_info = (line_info_t *)array_get(self->r->line_info, cursor_get_y(self));
    assert(line_info != NULL);
    cursor_set_x(self, line_info->line_length);
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
cursor_snap_to_end (line_editor_t *self) {
  line_info_t *line_info = (line_info_t *)array_get(self->r->line_info, cursor_get_y(self));

  unsigned int length    = line_info ? line_info->line_length : 0;
  if (cursor_get_x(self) > length) {
    cursor_set_x(self, length);
  }
}

void
cursor_select_left (line_editor_t *self) {
  cursor_select(self, SELECT_LEFT);
}

void
cursor_select_left_word (line_editor_t *self) {
  cursor_select(self, SELECT_LEFT_WORD);
}

void
cursor_select_right (line_editor_t *self) {
  cursor_select(self, SELECT_RIGHT);
}

void
cursor_select_right_word (line_editor_t *self) {
  cursor_select(self, SELECT_RIGHT_WORD);
}

void
cursor_select_up (line_editor_t *self) {
  if (!cursor_is_select_active(self)) {
    cursor_copy_to_select_anchor(self);
  }

  cursor_set_is_active(self, true);

  if (cursor_get_y(self) == 0) {
    cursor_set_select_offset(self, 0, 0);
    cursor_set_x(self, 0);
    return;
  }

  cursor_move_up(self);
  cursor_copy_to_select_offset(self);
}

void
cursor_select_down (line_editor_t *self) {
  if (!cursor_is_select_active(self)) {
    cursor_copy_to_select_anchor(self);
  }

  cursor_set_is_active(self, true);

  if (cursor_get_y(self) == self->r->num_lines - 1) {
    line_info_t *line_info = (line_info_t *)array_get(self->r->line_info, cursor_get_y(self));
    assert(line_info != NULL);
    cursor_set_select_offset(self, line_info->line_length, cursor_get_y(self));
    cursor_set_x(self, self->curs.select_offset.x);
    return;
  }

  cursor_move_down(self);
  cursor_copy_to_select_offset(self);
}

bool
cursor_is_select_ltr (line_editor_t *self) {
  return (
      self->curs.select_anchor.y < self->curs.select_offset.y ||
      (self->curs.select_anchor.y == self->curs.select_offset.y &&
        self->curs.select_anchor.x <= self->curs.select_offset.x)
    );
}

void
cursor_select_clear (line_editor_t *self) {
  cursor_set_is_active(self, false);
  cursor_set_select_offset(self, -1, -1);
  cursor_set_select_anchor(self, -1, -1);
}

bool
cursor_on_first_line (line_editor_t *self) {
  return cursor_get_y(self) == 0;
}

bool
cursor_on_first_col (line_editor_t *self) {
  return cursor_get_x(self) == 0;
}

bool
cursor_on_last_line (line_editor_t *self) {
  return cursor_get_y(self) == self->r->num_lines - 1;
}

bool
cursor_above_visible_window (line_editor_t *self) {
  return cursor_get_y(self) < cursor_get_row_off(self);
}

bool
cursor_below_visible_window (line_editor_t *self) {
  return cursor_get_y(self) >= cursor_get_row_off(self) + window_get_num_rows();
}

bool
cursor_left_of_visible_window (line_editor_t *self) {
  return cursor_get_x(self) < cursor_get_col_off(self);
}

bool
cursor_right_of_visible_window (line_editor_t *self) {
  return cursor_get_x(self) >= cursor_get_col_off(self) + (window_get_num_cols() - (line_pad + 1));
}

bool
cursor_in_cell_zero (line_editor_t *self) {
  return cursor_get_x(self) == 0 && cursor_get_y(self) == 0;
}

bool
cursor_not_at_row_begin (line_editor_t *self) {
  return cursor_get_x(self) > 0;
}
