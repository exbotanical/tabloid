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
  array_t       *line;
  buffer_t      *tmp_buffer;
  piece_table_t *pt;
} line_buffer_t;

line_buffer_t *line_buffer_init(char *initial);
void           line_buffer_free(line_buffer_t *self);
void           line_buffer_refresh(line_buffer_t *self);
void           line_buffer_get_line(line_buffer_t *self, unsigned int lineno, char *buffer);
void           line_buffer_get_all(line_buffer_t *self, char **buffer);
void  line_buffer_insert(line_buffer_t *self, int x, int y, char *insert_chars, void *metadata);
void  line_buffer_delete(line_buffer_t *self, int x, int y, void *metadata);
void *line_buffer_undo(line_buffer_t *self);
void *line_buffer_redo(line_buffer_t *self);
bool  line_buffer_dirty(line_buffer_t *self);

#endif /* LINE_BUFFER_H */
