#include "editor.h"

#include <aio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "exception.h"
#include "globals.h"

static void
editor_refresh_row (row_buffer_t *row) {
  // Count tabs - we need to malloc extra bytes for each
  int tabs = 0;
  for (unsigned int i = 0; i < row->raw_sz; i++) {
    if (row->raw[i] == '\t') {
      tabs++;
    }
  }

  // Update the render buffer, then redraw
  free(row->renderbuf);
  row->renderbuf = malloc(row->raw_sz + tabs * (editor.config.tab_sz - 1) + 1);

  unsigned int idx = 0;
  for (unsigned int i = 0; i < row->raw_sz; i++) {
    // Render tab
    if (row->raw[i] == '\t') {
      row->renderbuf[idx++] = ' ';

      while (idx % editor.config.tab_sz != 0) {
        row->renderbuf[idx++] = ' ';
      }
    } else {
      row->renderbuf[idx++] = row->raw[i];
    }
  }

  row->renderbuf[idx] = '\0';
  row->renderbuf_sz   = idx;
}

static void
editor_insert_row (int at, char *s, size_t len) {
  if (at < 0 || at > editor.buf.num_rows) {
    return;
  }

  // Make room at `at`
  editor.buf.rows = realloc(editor.buf.rows, sizeof(row_buffer_t) * (editor.buf.num_rows + 1));
  memmove(
    &editor.buf.rows[at + 1],
    &editor.buf.rows[at],
    sizeof(row_buffer_t) * (editor.buf.num_rows - at)
  );

  // Create new row
  editor.buf.rows[at].raw_sz = len;
  editor.buf.rows[at].raw    = malloc(len + 1);
  memcpy(editor.buf.rows[at].raw, s, len);
  editor.buf.rows[at].raw[len]     = '\0';
  editor.buf.rows[at].renderbuf    = NULL;
  editor.buf.rows[at].renderbuf_sz = 0;

  editor_refresh_row(&editor.buf.rows[at]);

  editor.buf.num_rows++;
  // dirty++
}

static void
editor_free_row (row_buffer_t *row) {
  free(row->renderbuf);
  free(row->raw);
}

static void
editor_row_concat (row_buffer_t *row, char *s, size_t len) {
  row->raw = realloc(row->raw, row->raw_sz + len + 1);
  memcpy(&row->raw[row->raw_sz], s, len);
  row->raw_sz           += len;
  row->raw[row->raw_sz]  = '\0';
  editor_refresh_row(row);
  // dirty++
}

static void
editor_del_row (int at) {
  if (at < 0 || at >= editor.buf.num_rows) {
    return;
  }
  editor_free_row(&editor.buf.rows[at]);
  memmove(
    &editor.buf.rows[at],
    &editor.buf.rows[at + 1],
    sizeof(row_buffer_t) * (editor.buf.num_rows - at - 1)
  );
  editor.buf.num_rows--;
  // dirty++
}

static void
editor_row_insert_char (row_buffer_t *row, unsigned int at, unsigned int c) {
  // Add 2 to account for null byte
  row->raw = realloc(row->raw, row->raw_sz + 2);
  memmove(&row->raw[at + 1], &row->raw[at], row->raw_sz - at + 1);
  row->raw_sz++;
  row->raw[at] = c;

  editor_refresh_row(row);
}

static void
editor_row_del_char (row_buffer_t *row, int at) {
  if (at < 0 || at >= row->raw_sz) {
    return;
  }

  memmove(&row->raw[at], &row->raw[at + 1], row->raw_sz - at);
  row->raw_sz--;
  editor_refresh_row(row);
  // dirty++
}

void
editor_insert_char (int c) {
  if (editor.curs.y == editor.buf.num_rows) {
    editor_insert_row(editor.buf.num_rows, "", 0);
  }
  editor_row_insert_char(&editor.buf.rows[editor.curs.y], editor.curs.x, c);
  editor.curs.x++;
}

void
editor_del_char (void) {
  // If past the end of the file...
  if (editor.curs.y == editor.buf.num_rows) {
    return;
  }

  // If at the beginning of the first line...
  if (editor.curs.x == 0 && editor.curs.y == 0) {
    return;
  }

  row_buffer_t *row = &editor.buf.rows[editor.curs.y];
  // If char to the left of the cursor...
  if (editor.curs.x > 0) {
    editor_row_del_char(row, editor.curs.x - 1);
    editor.curs.x--;
  } else {
    // We're at the beginning of the row
    editor.curs.x = editor.buf.rows[editor.curs.y - 1].raw_sz;
    editor_row_concat(&editor.buf.rows[editor.curs.y - 1], row->raw, row->raw_sz);
    editor_del_row(editor.curs.y);
    editor.curs.y--;
  }
}

void
editor_insert_newline (void) {
  // If at beginning of line, insert a new one
  if (editor.curs.x == 0) {
    editor_insert_row(editor.curs.y, "", 0);
  } else {
    // Otherwise, we're in the middle of a line and must split it in two
    row_buffer_t *row = &editor.buf.rows[editor.curs.y];
    editor_insert_row(
      editor.curs.y + 1,
      &row->raw[editor.curs.x],
      row->raw_sz - editor.curs.x
    );

    row                   = &editor.buf.rows[editor.curs.y];
    row->raw_sz           = editor.curs.x;
    row->raw[row->raw_sz] = '\0';

    editor_refresh_row(row);
  }

  editor.curs.y++;
  editor.curs.x = 0;
}

void
editor_init (void) {
  if (tty_get_window_sz(&editor.win.rows, &editor.win.cols) == -1) {
    panic("Failed call to get terminal window size\n");
  }

  editor.curs              = (cursor_t){.x = 0, .y = 0, .row = 0, .col = 0};
  editor.buf.num_rows      = 0;
  editor.renderx           = DEFAULT_LNPAD;
  editor.buf.rows          = NULL;

  editor.config.tab_sz     = DEFAULT_TAB_SZ;
  editor.config.ln_prefix  = DEFAULT_LINE_PREFIX;

  // Subtract 1 for the status bar
  editor.win.rows         -= 1;
  editor.sbar.msg[0]       = '\0';

  editor_insert_row(editor.curs.y, "", 0);
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
    editor_insert_row(editor.buf.num_rows, line, line_len);
  }

  free(line);
  fclose(fd);
}
