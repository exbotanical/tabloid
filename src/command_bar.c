#include "command_bar.h"

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "parser.h"

static void
command_bar_do_quit (line_editor_t* self, command_token_t* command) {
  if (line_buffer_dirty(editor.line_ed.r) && ((command->mods & TOKEN_MOD_OVERRIDE) != TOKEN_MOD_OVERRIDE)) {
    command_bar_set_message_mode(self, "No write since last change");
    return;
  } else {
    exit(0);
  }
}

static void
command_bar_do_write (line_editor_t* self, command_token_t* command) {
  // if dirty:
  // if new_name && exists:
  // Still dirty if write to another file?
}

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

  bool             is_override = false;
  command_token_t* command     = parser_parse_wrapped(line);

  switch (command->command) {
    case COMMAND_WRITE: {
      if (!editor.filepath) {
        command_bar_set_message_mode(self, "No file name");
      } else {
        ssize_t n_bytes = editor_save(editor.filepath);
        command_bar_set_message_mode(self, "Wrote %d bytes to %s", n_bytes, editor.filepath);
      }

      break;
    }
    case COMMAND_QUIT:
    case COMMAND_WRITE_QUIT: {
      command_bar_do_quit(self, command);
      break;
    }
    case COMMAND_INVALID: {
      // TODO: message level e.g. error, warn, info, etc
      command_bar_set_message_mode(self, "Unknown command");
      break;
    }
  }

  parser_command_token_free(command);
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
