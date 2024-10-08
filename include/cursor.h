#ifndef CURSOR_H
#define CURSOR_H

#include <stdbool.h>

#include "libutil/libutil.h"

/**
 * Cursor state
 */
typedef struct {
  // x coordinate of cursor
  unsigned int x;
  // y coordinate of cursor
  unsigned int y;
  // Row (y) offset used for scroll i.e. number of rows past the window size
  unsigned int row_off;
  // Column (x) offset used for scroll i.e. number of columns past the window size
  unsigned int col_off;
  // Cursor relative to the render buffer
  // render_x will likely equal x unless we've rendered something differently
  // than its underlying source (e.g. tabs)
  unsigned int render_x;
} cursor_t;

bool cursor_on_content_line(void);
bool cursor_on_first_line(void);
bool cursor_on_first_col(void);
bool cursor_above_visible_window(void);
bool cursor_below_visible_window(void);
bool cursor_left_of_visible_window(void);
bool cursor_right_of_visible_window(void);
bool cursor_in_cell_zero(void);
bool cursor_not_at_row_begin(void);

void cursor_move_down(void);
void cursor_move_up(void);
void cursor_move_left(void);
void cursor_move_right(void);
void cursor_move_top(void);
void cursor_move_visible_top(void);
void cursor_move_bottom(void);
void cursor_move_visible_bottom(void);
void cursor_move_begin(void);
void cursor_move_end(void);
void cursor_snap_to_end(void);
void cursor_move_right_word(void);
void cursor_move_left_word(void);

int  cursor_get_position(unsigned int *rows, unsigned int *cols);
void cursor_set_position(buffer_t *buf);

#endif /* CURSOR_H */
