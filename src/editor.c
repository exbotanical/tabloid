#include "editor.h"

#include <aio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "cursor.h"
#include "exception.h"
#include "globals.h"

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

    char           *data = malloc(1);
    size_t          sz   = sizeof(data);
    read_all_result ret  = read_all(fd, &data, &sz);
    fclose(fd);

    // TODO: status bar, not panic
    switch (ret) {
      case READ_ALL_INVALID: panic("an error occurred while reading %s\n", filepath); break;
      case READ_ALL_ERR: panic("a stream error occurred while reading %s\n", filepath); break;
      case READ_ALL_TOO_LARGE: panic("failed to read %s - input was too large\n", filepath); break;
      case READ_ALL_NOMEM: panic("ran out of memory while reading %s\n", filepath); break;
      case READ_ALL_OK: break;
    }

    line_buffer_insert(editor.line_ed.r, 0, 0, data, NULL);
    event_stack_clear(editor.line_ed.r->pt->undo_stack);
    editor.line_ed.r->pt->last_event_index = 0;
    editor.line_ed.r->pt->last_event       = PT_SENTINEL;
  }

  editor.filepath = filepath;
}

// TODO: Logging
void
editor_save (const char *filepath) {
  unsigned int sz = piece_table_size(editor.line_ed.r->pt);
  char         s[sz];

  FILE *fd = fopen(filepath, "wb+");
  if (!fd) {
    // TODO: No panic - just status bar update
    panic("failed to open file %s\n", filepath);
  }

  piece_table_render(editor.line_ed.r->pt, 0, sz, s);
  write_all_result ret = write_all(fd, s);
  fclose(fd);

  switch (ret) {
    case WRITE_ALL_INVALID:
      panic("an error occurred while writing %s - invalid data or file descriptor\n", filepath);
      break;
    case WRITE_ALL_ERR: panic("an error occurred while writing %s\n", filepath); break;
    // TODO: Actually handle this somehow. Either use a swapfile or atomic op so we can rollback the file.
    case WRITE_ALL_INCOMPLETE:
      panic("an error occurred while writing %s - incomplete write. sorry we screwed up your file oops\n", filepath);
      break;
    case WRITE_ALL_OK: break;
  }
}
