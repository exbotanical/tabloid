#include "command_bar.h"

#include <assert.h>
#include <stdarg.h>
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

  editor.cmode       = CB_INPUT;
  memset(editor.cbar_msg, 0, sizeof(editor.cbar_msg));
}

void
command_bar_process_command (line_editor_t* self) {
  line_info_t* row = (line_info_t*)array_get(self->r->line_info, 0);
  assert(row != NULL);

  char line[row->line_length];
  line_buffer_get_line(editor.c_bar.r, 0, line);

  /*
  L = 1
  w, q

  L = 2
  w, q - mod ! - unknown

  L = n
  w <fpath> - mod ! - unknown
  */

  // TODO: parser
  if (s_equals(line, "q")) {
    if (line_buffer_dirty(editor.line_ed.r)) {
      command_bar_set_message_mode(self, "No write since last change");
      return;
    } else {
      exit(0);
    }
  } else if (s_equals(line, "q!")) {
    exit(0);
  } else if (s_equals(line, "w")) {
    if (!editor.filepath) {
      command_bar_set_message_mode(self, "No file name");
    } else {
      int n_bytes = editor_save(editor.filepath);
      command_bar_set_message_mode(self, "Wrote %d bytes to %s", n_bytes, editor.filepath);
    }
    return;
  } else {
    // TODO: message level e.g. error, warn, info, etc
    command_bar_set_message_mode(self, "Unknown command");
    return;
  }

  command_bar_clear(self);
}

void
command_bar_set_msg (const char* fmt, va_list va) {
  vsnprintf(editor.cbar_msg, sizeof(editor.cbar_msg), fmt, va);
}

void
command_bar_set_message_mode (line_editor_t* self, const char* fmt, ...) {
  command_bar_clear(self);

  va_list ap;
  va_start(ap, fmt);
  command_bar_set_msg(fmt, ap);
  va_end(ap);

  editor.cmode = CB_MESSAGE;
}
