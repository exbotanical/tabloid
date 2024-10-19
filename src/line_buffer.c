#include "line_buffer.h"

#include <stdlib.h>

static void
render_state_reset (render_state_t *self) {
  buffer_free(self->line_buffer);
  array_free(self->line_starts, free);
  self->line_starts = array_init(0);
  self->line_buffer = buffer_init(NULL);
}

render_state_t *
render_state_init (void) {
  render_state_t *self = malloc(sizeof(render_state_t));
  self->line_starts    = array_init(0);
  self->line_buffer    = buffer_init(NULL);
  return self;
}

void
render_state_free (render_state_t *self) {
  buffer_free(self->line_buffer);
  array_free(self->line_starts, free);
  free(self);
}

void
render_state_refresh (render_state_t *self, piece_table_t *pt) {
  unsigned int doc_size = piece_table_size(pt);
  render_state_reset(self, dop);

  unsigned int offset_chars = 0;
  unsigned int line_start   = 0;
  unsigned int num_lines    = 0;

  for (; offset_chars < doc_size;) {
    char c[2];
    piece_table_render(pt, offset_chars++, 1, c);
    buffer_append(self->line_buffer, c);

    if (c[0] == '\n') {
      // TODO:
      self->line_starts[num_lines] = line_start;
      line_start                   = offset_chars;
      num_lines++;
    }
  }

  self->line_starts[num_lines++] = line_start;
  self->line_starts[num_lines]   = offset_chars;
}
