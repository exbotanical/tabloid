#include "line_buffer.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static line_info_t *
line_info_init (unsigned int start, unsigned int length) {
  line_info_t *self = malloc(sizeof(line_info_t));
  self->line_start  = start;
  self->line_length = length;

  return self;
}

static void
line_info_free (line_info_t *self) {
  free(self);
}

static void
render_state_reset (render_state_t *self) {
  buffer_free(self->tmp_buffer);
  array_free(self->line_info, (free_fn *)line_info_free);  // TODO: optimize by caching
  array_free(self->line_buffer, NULL);

  self->tmp_buffer  = buffer_init(NULL);
  self->line_info   = array_init();
  self->line_buffer = array_init();
}

render_state_t *
render_state_init (char *initial) {
  render_state_t *self = malloc(sizeof(render_state_t));
  self->line_info      = array_init();
  self->line_buffer    = array_init();
  self->tmp_buffer     = buffer_init(NULL);
  self->pt             = piece_table_init();

  piece_table_setup(self->pt, initial);

  return self;
}

void
render_state_free (render_state_t *self) {
  piece_table_free(self->pt);
  buffer_free(self->tmp_buffer);
  array_free(self->line_info, NULL);
  array_free(self->line_buffer, NULL);
  free(self);
}

void
render_state_refresh (render_state_t *self) {
  render_state_reset(self);

  unsigned int doc_size     = piece_table_size(self->pt);

  unsigned int offset_chars = 0;
  unsigned int line_start   = 0;
  unsigned int num_lines    = doc_size ? 1 : 0;
  unsigned int line_length  = 0;

  for (; offset_chars < doc_size;) {
    line_length++;
    char c[2];

    piece_table_render(self->pt, offset_chars++, 1, c);
    buffer_append(self->tmp_buffer, c);

    if (c[0] == '\n') {
      line_info_t *li = line_info_init(line_start, line_length - 1);
      array_push(self->line_info, (void *)li);

      line_start  = offset_chars;
      line_length = 0;
      num_lines++;
    }
  }

  if (doc_size) {
    line_info_t *li = line_info_init(line_start, line_length);
    array_push(self->line_info, (void *)li);
  }
}

// void
// render_state_refresh (render_state_t *self) {
//   unsigned int doc_size = piece_table_size(self->pt);
//   render_state_reset(self);

//   unsigned int offset_chars = 0;
//   unsigned int line_start   = 0;
//   unsigned int num_lines    = 0;

//   for (; offset_chars < doc_size;) {
//     char c[2];
//     piece_table_render(self->pt, offset_chars++, 1, c);
//     buffer_append(self->line_buffer, c);

//     if (c[0] == '\n') {
//       array_push(self->line_starts, (void *)line_start);
//       line_start = offset_chars;
//     }
//   }

//   array_push(self->line_starts, (void *)line_start);
//   array_push(self->line_starts, (void *)offset_chars);
// }

void
render_state_get_line (render_state_t *self, unsigned int lineno, char *buffer) {
  unsigned int num_lines = array_size(self->line_info);
  assert(num_lines > lineno);  // TODO:

  line_info_t *line_info   = (line_info_t *)array_get(self->line_info, lineno);
  unsigned int line_start  = line_info->line_start;
  unsigned int line_length = line_info->line_length;

  char *line = s_substr(buffer_state(self->tmp_buffer), line_start, line_start + line_length, false);

  memcpy(buffer, line, strlen(line) + 1);
  buffer[strlen(line) + 1] = '\0';

  free(line);
}

static unsigned int
get_absolute_index (render_state_t *self, int x, int y) {
  return ((line_info_t *)array_get(self->line_info, y))->line_start + x;
}

void
render_state_insert (render_state_t *self, int x, int y, char *insert_chars) {
  unsigned int absolute_index = get_absolute_index(self, x, y);
  piece_table_insert(self->pt, absolute_index, insert_chars);
  render_state_refresh(self);
}

void
render_state_delete (render_state_t *self, int x, int y) {
  unsigned int absolute_index = get_absolute_index(self, x, y);
  piece_table_delete(self->pt, absolute_index, 1);
  render_state_refresh(self);
}
