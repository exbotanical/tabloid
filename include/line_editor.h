#ifndef LINE_EDITOR_H
#define LINE_EDITOR_H

#include "line_buffer.h"

typedef struct {
  unsigned int x;
  unsigned int y;
} coords_t;

/**
 * Cursor state
 */
typedef struct {
  coords_t select_anchor;
  coords_t select_offset;

  // TODO: use coords_t
  // x coordinate of cursor
  unsigned int x;
  // y coordinate of cursor
  unsigned int y;
  // Row (y) offset used for scroll i.e. number of rows past the window size
  unsigned int row_off;
  // Column (x) offset used for scroll i.e. number of columns past the window size
  unsigned int col_off;

  bool select_active;
} cursor_t;

typedef struct {
  cursor_t       curs;
  line_buffer_t* r;
} line_editor_t;

void line_editor_init(line_editor_t* self);
void line_editor_insert_char(line_editor_t* self, int c);
void line_editor_delete_char(line_editor_t* self);
void line_editor_delete_line_before_x(line_editor_t* self);
void line_editor_insert_newline(line_editor_t* self);
void line_editor_insert(line_editor_t* self, char* s);
void line_editor_undo(line_editor_t* self);
void line_editor_redo(line_editor_t* self);

#endif /* LINE_EDITOR_H */
