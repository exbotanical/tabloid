#ifndef CURSOR_H
#define CURSOR_H

#include <stdbool.h>

#include "libutil/libutil.h"
#include "line_editor.h"

// TODO: Move me
ssize_t cursor_get_position(size_t *rows, size_t *cols);

static inline size_t
cursor_get_x (line_editor_t *self) {
  return self->curs.x;
}

static inline size_t
cursor_get_y (line_editor_t *self) {
  return self->curs.y;
}

static inline size_t
cursor_get_anchor_x (line_editor_t *self) {
  return self->curs.select_anchor.x;
}

static inline size_t
cursor_get_anchor_y (line_editor_t *self) {
  return self->curs.select_anchor.y;
}

static inline size_t
cursor_get_offset_x (line_editor_t *self) {
  return self->curs.select_offset.x;
}

// TODO: PAY CC!!!!
static inline size_t
cursor_get_offset_y (line_editor_t *self) {
  return self->curs.select_offset.y;
}

static inline size_t
cursor_get_row_off (line_editor_t *self) {
  return self->curs.row_off;
}

static inline size_t
cursor_get_col_off (line_editor_t *self) {
  return self->curs.col_off;
}

static inline bool
cursor_is_select_active (line_editor_t *self) {
  return self->curs.select_active;
}

static inline void
cursor_set_x (line_editor_t *self, size_t x) {
  self->curs.x = x;
}

static inline size_t
cursor_inc_x (line_editor_t *self) {
  self->curs.x++;
  return cursor_get_x(self);
}

static inline size_t
cursor_dec_x (line_editor_t *self) {
  self->curs.x--;
  return cursor_get_x(self);
}

static inline void
cursor_set_y (line_editor_t *self, size_t y) {
  self->curs.y = y;
}

static inline size_t
cursor_inc_y (line_editor_t *self) {
  self->curs.y++;
  return cursor_get_y(self);
}

static inline size_t
cursor_dec_y (line_editor_t *self) {
  self->curs.y--;
  return cursor_get_y(self);
}

static inline void
cursor_set_xy (line_editor_t *self, size_t x, size_t y) {
  cursor_set_x(self, x);
  cursor_set_y(self, y);
}

static inline void
cursor_set_row_off (line_editor_t *self, size_t row_off) {
  self->curs.row_off = row_off;
}

static inline void
cursor_set_col_off (line_editor_t *self, size_t col_off) {
  self->curs.col_off = col_off;
}

static inline void
cursor_set_is_active (line_editor_t *self, bool next) {
  self->curs.select_active = next;
}

void cursor_set_position(line_editor_t *self, buffer_t *buf);
void cursor_set_position_command_bar(line_editor_t *self, buffer_t *buf);

cursor_t *cursor_create_copy(line_editor_t *self);

void cursor_move_down(line_editor_t *self);
void cursor_move_up(line_editor_t *self);
void cursor_move_left(line_editor_t *self);
void cursor_move_left_word(line_editor_t *self);
void cursor_move_right(line_editor_t *self);
void cursor_move_right_word(line_editor_t *self);
void cursor_move_top(line_editor_t *self);
void cursor_move_visible_top(line_editor_t *self);
void cursor_move_bottom(line_editor_t *self);
void cursor_move_visible_bottom(line_editor_t *self);
void cursor_move_begin(line_editor_t *self);
void cursor_move_end(line_editor_t *self);
void cursor_snap_to_end(line_editor_t *self);

void cursor_select_left(line_editor_t *self);
void cursor_select_left_word(line_editor_t *self);
void cursor_select_right(line_editor_t *self);
void cursor_select_right_word(line_editor_t *self);
void cursor_select_up(line_editor_t *self);
void cursor_select_down(line_editor_t *self);
bool cursor_is_select_ltr(line_editor_t *self);
void cursor_select_clear(line_editor_t *self);

bool cursor_on_first_line(line_editor_t *self);
bool cursor_on_first_col(line_editor_t *self);
bool cursor_on_last_line(line_editor_t *self);
bool cursor_above_visible_window(line_editor_t *self);
bool cursor_below_visible_window(line_editor_t *self);
bool cursor_left_of_visible_window(line_editor_t *self);
bool cursor_right_of_visible_window(line_editor_t *self);
bool cursor_in_cell_zero(line_editor_t *self);
bool cursor_not_at_row_begin(line_editor_t *self);

#endif /* CURSOR_H */
