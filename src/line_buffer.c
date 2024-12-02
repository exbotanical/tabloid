#include "line_buffer.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "xmalloc.h"

static line_info_t *
line_info_init (unsigned int start, unsigned int length) {
  line_info_t *self = xmalloc(sizeof(line_info_t));
  self->line_start  = start;
  self->line_length = length;

  return self;
}

static void
line_info_free (line_info_t *self) {
  free(self);
}

static void
line_buffer_reset (line_buffer_t *self) {
  buffer_free(self->tmp_buffer);
  array_free(self->line_info, (free_fn *)line_info_free);  // TODO: optimize
  array_free(self->line, NULL);

  self->tmp_buffer = buffer_init(NULL);
  self->line_info  = array_init();
  self->num_lines  = 1;
  self->line       = array_init();
}

line_buffer_t *
line_buffer_init (char *initial) {
  line_buffer_t *self = xmalloc(sizeof(line_buffer_t));
  self->line_info     = array_init();
  self->num_lines     = 1;
  self->line          = array_init();
  self->tmp_buffer    = buffer_init(NULL);
  self->pt            = piece_table_init();

  piece_table_setup(self->pt, initial);

  return self;
}

void
line_buffer_free (line_buffer_t *self) {
  piece_table_free(self->pt);
  buffer_free(self->tmp_buffer);
  array_free(self->line_info, NULL);
  array_free(self->line, NULL);
  free(self);
}

void
line_buffer_refresh (line_buffer_t *self) {
  line_buffer_reset(self);

  unsigned int doc_size     = piece_table_size(self->pt);

  unsigned int offset_chars = 0;
  unsigned int line_start   = 0;
  unsigned int num_lines    = 1;
  unsigned int line_length  = 0;

  if (doc_size == 0) {
    line_info_t *li = line_info_init(0, 0);
    array_push(self->line_info, (void *)li);
    self->num_lines = 1;
    return;
  }

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

  self->num_lines = num_lines;
}

void
line_buffer_get_line (line_buffer_t *self, unsigned int lineno, char *buffer) {
  assert(self->num_lines > lineno);

  line_info_t *line_info   = (line_info_t *)array_get(self->line_info, lineno);
  unsigned int line_start  = line_info->line_start;
  unsigned int line_length = line_info->line_length;

  if (line_length == 0) {
    buffer = "";
    return;
  }

  char *line = s_substr(buffer_state(self->tmp_buffer), line_start, line_start + line_length, false);

  memcpy(buffer, line, strlen(line) + 1);
  buffer[strlen(line) + 1] = '\0';

  free(line);
}

void
line_buffer_get_all (line_buffer_t *self, char **buffer) {
  unsigned int sz = piece_table_size(self->pt);
  char         s[sz];

  piece_table_render(self->pt, 0, sz, s);
  *buffer = s;
}

static unsigned int
get_absolute_index (line_buffer_t *self, int x, int y) {
  if (array_size(self->line_info) == 0) {
    return 0;
  }
  return ((line_info_t *)array_get(self->line_info, y))->line_start + x;
}

// TODO: store metadata only when needed (when dealing with a group)
// Perhaps we return a bool or enum indicating when the metadata wasn't needed
// so the caller can call free immediately. Storing a cursor on the heap for
// every single piece table update is a bit heavy-handed.
void
line_buffer_insert (line_buffer_t *self, int x, int y, char *insert_chars, void *metadata) {
  unsigned int absolute_index = get_absolute_index(self, x, y);
  piece_table_insert(self->pt, absolute_index, insert_chars, metadata);
  line_buffer_refresh(self);
}

void
line_buffer_delete (line_buffer_t *self, int x, int y, void *metadata) {
  unsigned int absolute_index = get_absolute_index(self, x, y);
  piece_table_delete(self->pt, absolute_index, 1, PT_DELETE, metadata);
  line_buffer_refresh(self);
}

void *
line_buffer_undo (line_buffer_t *self) {
  void *metadata = piece_table_undo(self->pt);
  line_buffer_refresh(self);
  return metadata;
}

void *
line_buffer_redo (line_buffer_t *self) {
  void *metadata = piece_table_redo(self->pt);
  line_buffer_refresh(self);
  return metadata;
}

bool
line_buffer_dirty (line_buffer_t *self) {
  return piece_table_dirty(self->pt);
}

void
line_buffer_dirty_reset (line_buffer_t *self) {
  piece_table_dirty_reset(self->pt);
}
