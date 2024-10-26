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
  render_state_insert(editor.r, editor.curs.x, editor.curs.y, s);
}

void
editor_insert_char (int c) {
  char cp[1];
  cp[0] = c;
  cp[1] = '\0';
  render_state_insert(editor.r, editor.curs.x, editor.curs.y, cp);

  editor.curs.x++;
}

void
editor_del_char (void) {
  // If past the end of the file...
  if (!cursor_on_content_line()) {
    return;
  }

  // If at the beginning of the first line...
  if (cursor_in_cell_zero()) {
    return;
  }

  // If char to the left of the cursor...
  if (cursor_not_at_row_begin()) {
    render_state_delete(editor.r, editor.curs.x - 1, editor.curs.y);
    editor.curs.x--;
  } else {
    line_info_t *row = (line_info_t *)array_get(editor.r->line_info, editor.curs.y - 1);
    // We're at the beginning of the row
    editor.curs.x = row->line_length;
    render_state_delete(editor.r, -1, editor.curs.y);
    editor.curs.y--;
  }
}

void
editor_insert_newline (void) {
  char nl[1];
  nl[0] = '\n';
  nl[1] = '\0';
  render_state_insert(editor.r, editor.curs.x, editor.curs.y, nl);
  editor.curs.y++;
  editor.curs.x = 0;
}

void
editor_init (void) {
  if (tty_get_window_sz(&editor.win.rows, &editor.win.cols) == -1) {
    panic("Failed call to get terminal window size\n");
  }

  editor.curs = (cursor_t){
    .x             = 0,
    .y             = 0,
    .row_off       = 0,
    .col_off       = 0,
    .render_x      = DEFAULT_LNPAD,
    .select_active = false,
    .select_anchor = -1,
    .select_offset = -1,
  };

  editor.conf.tab_sz     = DEFAULT_TAB_SZ;
  editor.conf.ln_prefix  = DEFAULT_LINE_PREFIX;

  // Subtract for the status bar
  editor.win.rows       -= 2;
  editor.s_bar.msg[0]    = '\0';
  editor.c_bar.msg[0]    = '\0';

  editor.r               = render_state_init(NULL);

  editor_insert("");
}

void
editor_open (const char *filename) {
  FILE *fd = fopen(filename, "r");
  if (!fd) {
    panic("fopen");
  }

  char   *line         = NULL;
  size_t  max_line_len = 0;
  ssize_t line_len;

  while ((line_len = getline(&line, &max_line_len, fd)) != -1) {
    while (line_len > 0 && (line[line_len - 1] == '\n' || line[line_len - 1] == '\r')) {
      line_len--;
    }
    editor_insert(line);
  }

  free(line);
  fclose(fd);
}
