#include "editor.h"

#include <aio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "cursor.h"
#include "exception.h"
#include "globals.h"
#include "xmalloc.h"

static void
editor_update_file_state_on_write (const char *filepath) {
  if (!editor.filepath) {
    editor.filepath = s_copy(filepath);
    line_buffer_dirty_reset(editor.line_ed.r);
  } else {
    // Only clear dirty flag if we're actually writing to the current file.
    if (s_equals(filepath, editor.filepath)) {
      line_buffer_dirty_reset(editor.line_ed.r);
    }
  }
}

void
editor_init (editor_t *self) {
  if (tty_get_window_size(&(self->win.rows), &(self->win.cols)) == -1) {
    panic("Failed call to get terminal window size\n");
  }

  self->conf.tab_sz               = DEFAULT_TAB_SZ;
  self->conf.ln_prefix            = DEFAULT_LINE_PREFIX;

  // Subtract for the status bar
  self->win.rows                 -= 2;
  self->s_bar.left_component[0]   = '\0';
  self->s_bar.right_component[0]  = '\0';

  line_editor_init(&self->c_bar);
  line_editor_init(&self->line_ed);

  self->filepath = NULL;

  mode_chmod(EDIT_MODE);
}

void
editor_free (editor_t *self) {
  line_buffer_free(self->c_bar.r);
  line_buffer_free(self->line_ed.r);
}

// TODO: Handle very large files
void
editor_open (const char *filepath) {
  if (file_exists(filepath)) {
    FILE *fd = fopen(filepath, "rb+");
    if (!fd) {
      panic("failed to open file %s\n", filepath);
    }

    size_t             sz;
    char              *data = xmalloc(1);
    io_read_all_result ret  = io_read_all(fd, &data, &sz);
    fclose(fd);

    // TODO: status bar, not panic
    switch (ret) {
      case IO_READ_ALL_INVALID: panic("an error occurred while reading %s\n", filepath); break;
      case IO_READ_ALL_ERR: panic("a stream error occurred while reading %s\n", filepath); break;
      case IO_READ_ALL_TOO_LARGE:
        panic("failed to read %s - input was too large\n", filepath);
        break;
      case IO_READ_ALL_NOMEM: panic("ran out of memory while reading %s\n", filepath); break;
      case IO_READ_ALL_OK: break;
    }

    line_buffer_insert(editor.line_ed.r, 0, 0, data, NULL);
    event_stack_clear(editor.line_ed.r->pt->undo_stack);
    editor.line_ed.r->pt->last_event_index = 0;
    editor.line_ed.r->pt->last_event       = PT_SENTINEL;
  }

  editor.filepath = filepath;
}

// TODO: Logging
int
editor_save (const char *filepath) {
  unsigned int sz = piece_table_size(editor.line_ed.r->pt);
  char         s[sz];

  FILE *fd = fopen(filepath, "wb+");
  if (!fd) {
    // TODO: No panic - just status bar update
    panic("failed to open file %s\n", filepath);
  }

  piece_table_render(editor.line_ed.r->pt, 0, sz, s);

  size_t              n_bytes;
  io_write_all_result ret = io_write_all(fd, s, &n_bytes);
  fclose(fd);

  switch (ret) {
    case IO_WRITE_ALL_INVALID:
      panic("an error occurred while writing %s - invalid data or file descriptor\n", filepath);
      break;
    case IO_WRITE_ALL_ERR: panic("an error occurred while writing %s\n", filepath); break;
    // TODO: Actually handle this somehow. Either use a swapfile or atomic op so we can rollback the file.
    case IO_WRITE_ALL_INCOMPLETE:
      panic("an error occurred while writing %s - incomplete write. sorry we screwed up your file oops\n", filepath);
      break;
    case IO_WRITE_ALL_OK: break;
  }

  editor_update_file_state_on_write(filepath);

  return (int)n_bytes;
}
