#include "command_bar.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"

void
command_bar_clear (line_editor_t* self) {
  line_buffer_free(self->r);
  self->r            = line_buffer_init(NULL);
  self->curs.x       = 0;
  self->curs.y       = 0;
  self->curs.col_off = 0;
  self->curs.row_off = 0;
}

void
command_bar_process_command (line_editor_t* self) {
  line_info_t* row = (line_info_t*)array_get(self->r->line_info, 0);
  assert(row != NULL);

  char line[row->line_length];
  line_buffer_get_line(editor.c_bar.r, 0, line);

  if (strncmp(line, "q", strlen(line)) == 0) {
    // TODO: Dirty mode, write+quit, etc
    exit(0);
  }

  command_bar_clear(self);
}
