#ifndef LINE_BUFFER_H
#define LINE_BUFFER_H

#include "libutil/libutil.h"
#include "piece_table.h"

typedef struct {
  unsigned int line_start;
  unsigned int line_length;
} line_info_t;

typedef struct {
  // array_t<line_info_t>
  array_t       *line_info;
  unsigned int   num_lines;
  // array_t<char*>
  array_t       *line_buffer;
  buffer_t      *tmp_buffer;
  piece_table_t *pt;
} render_state_t;

render_state_t *render_state_init(char *initial);
void            render_state_free(render_state_t *self);
void            render_state_refresh(render_state_t *self);
void render_state_get_line(render_state_t *self, unsigned int lineno, char *buffer);
void render_state_insert(render_state_t *self, int x, int y, char *insert_chars);
void render_state_delete(render_state_t *self, int x, int y);

#endif /* LINE_BUFFER_H */
