#include "editor.h"

#include <aio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "exception.h"
#include "globals.h"

// TODO: refactor for better separation of concerns
void
editor_insert (char *s) {
  line_buffer_insert(editor.r, cursor_get_x(), cursor_get_y(), s, cursor_create_copy());
}

void
editor_insert_char (int c) {
  char cp[1];
  cp[0] = c;
  cp[1] = '\0';
  line_buffer_insert(editor.r, cursor_get_x(), cursor_get_y(), cp, cursor_create_copy());

  cursor_inc_x();
}

void
editor_delete_char (void) {
  // If at the beginning of the first line...
  if (cursor_in_cell_zero()) {
    return;
  }

  cursor_t *curs = cursor_create_copy();
  // If char to the left of the cursor...
  if (cursor_not_at_row_begin()) {
    line_buffer_delete(editor.r, cursor_get_x() - 1, cursor_get_y(), curs);
    cursor_dec_x();
  } else {
    line_info_t *row = (line_info_t *)array_get(editor.r->line_info, cursor_get_y() - 1);
    // We're at the beginning of the row
    cursor_set_x(row->line_length);
    line_buffer_delete(editor.r, -1, cursor_get_y(), curs);
    cursor_dec_y();
  }
}

void
editor_delete_line_before_x (void) {
  cursor_t *curs = cursor_create_copy();

  while (cursor_get_x() != 0) {
    cursor_dec_x();
    line_buffer_delete(editor.r, cursor_get_x(), cursor_get_y(), curs);
  }

  cursor_set_x(0);
}

void
editor_insert_newline (void) {
  char nl[1];
  nl[0] = '\n';
  nl[1] = '\0';
  line_buffer_insert(editor.r, cursor_get_x(), cursor_get_y(), nl, cursor_create_copy());
  cursor_inc_y();
  cursor_set_x(0);
}

// editor_open
void
editor_init (void) {
  if (tty_get_window_size(&editor.win.rows, &editor.win.cols) == -1) {
    panic("Failed call to get terminal window size\n");
  }

  editor.curs = (cursor_t){
    .x             = 0,
    .y             = 0,
    .row_off       = 0,
    .col_off       = 0,
    .select_active = false,
    .select_anchor = -1,
    .select_offset = -1,
  };

  editor.conf.tab_sz               = DEFAULT_TAB_SZ;
  editor.conf.ln_prefix            = DEFAULT_LINE_PREFIX;

  // Subtract for the status bar
  editor.win.rows                 -= 2;
  editor.s_bar.left_component[0]   = '\0';
  editor.s_bar.right_component[0]  = '\0';
  editor.c_bar.buf                 = line_buffer_init(NULL);

  editor.r                         = line_buffer_init(NULL);

  editor.filepath                  = NULL;

  mode_chmod(EDIT_MODE);
}

// TODO: Handle very large files
void
editor_open (const char *filepath) {
  if (file_exists(filepath)) {
    FILE *fd = fopen(filepath, "rb+");
    if (!fd) {
      panic("failed to open file %s\n", filepath);
    }

    char           *data = malloc(0);
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

    line_buffer_insert(editor.r, 0, 0, data, NULL);
  }

  editor.filepath = filepath;
}

// TODO: Handle very large files
// TODO: Logging
void
editor_save (const char *filepath) {
  unsigned int sz = piece_table_size(editor.r->pt);
  char         s[sz];

  FILE *fd = fopen(filepath, "wb+");
  if (!fd) {
    // TODO: No panic - just status bar update
    panic("failed to open file %s\n", filepath);
  }

  piece_table_render(editor.r->pt, 0, sz, s);
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

void
editor_undo (void) {
  cursor_t *old_curs = (cursor_t *)line_buffer_undo(editor.r);
  if (old_curs) {
    cursor_set_xy(old_curs->x, old_curs->y);
  }
}

// TODO: Need to implement shift key when not an escape sequence
void
editor_redo (void) {
  cursor_t *old_curs = (cursor_t *)line_buffer_redo(editor.r);
  if (old_curs) {
    cursor_set_xy(old_curs->x, old_curs->y);
  }
}
