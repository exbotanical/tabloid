#include "line_buffer.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void
render_state_reset (render_state_t *self) {
  buffer_free(self->line_buffer);
  array_free(self->line_starts, NULL);
  self->line_starts = array_init();
  self->line_buffer = buffer_init(NULL);
}

render_state_t *
render_state_init (char *initial) {
  render_state_t *self = malloc(sizeof(render_state_t));
  self->line_starts    = array_init();
  self->line_buffer    = buffer_init(NULL);
  self->pt             = piece_table_init();

  piece_table_setup(self->pt, initial);

  return self;
}

void
render_state_free (render_state_t *self) {
  piece_table_free(self->pt);
  buffer_free(self->line_buffer);
  array_free(self->line_starts, NULL);
  free(self);
}

void
render_state_refresh (render_state_t *self) {
  unsigned int doc_size = piece_table_size(self->pt);
  render_state_reset(self);

  unsigned int offset_chars = 0;
  unsigned int line_start   = 0;
  unsigned int num_lines    = 0;

  for (; offset_chars < doc_size;) {
    char c[2];
    piece_table_render(self->pt, offset_chars++, 1, c);
    buffer_append(self->line_buffer, c);

    if (c[0] == '\n') {
      array_push(self->line_starts, (void *)line_start);
      line_start = offset_chars;
    }
  }

  array_push(self->line_starts, (void *)line_start);
  array_push(self->line_starts, (void *)offset_chars);
}

void
render_state_get_line (render_state_t *self, unsigned int lineno, char *buffer) {
  unsigned int num_line_starts = array_size(self->line_starts);
  // We don't actually process the last line start - it's just there so we can grab an offset with the lineno+1 lookahead
  assert(num_line_starts > lineno + 1);  // TODO:

  unsigned int line_start = (unsigned int)array_get(self->line_starts, lineno);
  unsigned int line_length = (unsigned int)array_get(self->line_starts, lineno + 1) - line_start;

  char *line = s_substr(
    buffer_state(self->line_buffer),
    line_start,
    line_start + line_length - (num_line_starts == lineno + 2 ? 0 : 1),  // excise newline
    false
  );
  memcpy(buffer, line, strlen(line) + 1);
  buffer[strlen(line) + 1] = '\0';

  free(line);
}

char *
render_state_get_line_alt (render_state_t *self, unsigned int lineno) {
  unsigned int num_line_starts = array_size(self->line_starts);
  // We don't actually process the last line start - it's just there so we can grab an offset with the lineno+1 lookahead
  assert(num_line_starts != lineno - 1);  // TODO:
  unsigned int line_start = (unsigned int)array_get(self->line_starts, lineno);

  unsigned int line_length = (unsigned int)array_get(self->line_starts, lineno + 1) - line_start;

  return s_substr(
    buffer_state(self->line_buffer),
    line_start,
    line_start + line_length - (num_line_starts == lineno + 2 ? 0 : 1),  // excise newline
    false
  );
}

static unsigned int
get_absolute_index (render_state_t *self, int x, int y) {
  return (unsigned int)array_get(self->line_starts, y) + x;
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
