#include "line_editor.h"

#include <aio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cursor.h"
#include "exception.h"
#include "globals.h"

void
line_editor_init (line_editor_t *self) {
  self->curs = (cursor_t){
    .x             = 0,
    .y             = 0,
    .row_off       = 0,
    .col_off       = 0,
    .select_active = false,
    .select_anchor = -1,
    .select_offset = -1,
  };

  self->r = line_buffer_init(NULL);
}

void
line_editor_insert (line_editor_t *self, char *s) {
  line_buffer_insert(self->r, cursor_get_x(self), cursor_get_y(self), s, cursor_create_copy(self));
}

void
line_editor_insert_char (line_editor_t *self, ssize_t c) {
  char cp[1];
  cp[0] = c;
  cp[1] = '\0';
  line_buffer_insert(self->r, cursor_get_x(self), cursor_get_y(self), cp, cursor_create_copy(self));

  cursor_inc_x(self);
}

void
line_editor_delete_char (line_editor_t *self) {
  // If at the beginning of the first line...
  if (cursor_in_cell_zero(self)) {
    return;
  }

  cursor_t *curs = cursor_create_copy(self);
  // If char to the left of the cursor...
  if (cursor_not_at_row_begin(self)) {
    line_buffer_delete(self->r, cursor_get_x(self) - 1, cursor_get_y(self), curs);
    cursor_dec_x(self);
  } else {
    line_info_t *row = (line_info_t *)array_get(self->r->line_info, cursor_get_y(self) - 1);
    // We're at the beginning of the row
    cursor_set_x(self, row->line_length);
    line_buffer_delete(self->r, -1, cursor_get_y(self), curs);
    cursor_dec_y(self);
  }
}

void
line_editor_delete_line_before_x (line_editor_t *self) {
  cursor_t *curs = cursor_create_copy(self);

  while (cursor_get_x(self) != 0) {
    cursor_dec_x(self);
    line_buffer_delete(self->r, cursor_get_x(self), cursor_get_y(self), curs);
  }

  cursor_set_x(self, 0);
}

void
line_editor_insert_newline (line_editor_t *self) {
  char nl[1];
  nl[0] = '\n';
  nl[1] = '\0';
  line_buffer_insert(self->r, cursor_get_x(self), cursor_get_y(self), nl, cursor_create_copy(self));
  cursor_inc_y(self);
  cursor_set_x(self, 0);
}

void
line_editor_undo (line_editor_t *self) {
  cursor_t *old_curs = (cursor_t *)line_buffer_undo(self->r);
  if (old_curs) {
    cursor_set_xy(self, old_curs->x, old_curs->y);
  }
}

// TODO: Need to implement shift key when not an escape sequence
void
line_editor_redo (line_editor_t *self) {
  cursor_t *old_curs = (cursor_t *)line_buffer_redo(self->r);
  if (old_curs) {
    cursor_set_xy(self, old_curs->x, old_curs->y);
  }
}
