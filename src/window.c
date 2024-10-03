#include "window.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "cursor.h"
#include "editor.h"
#include "globals.h"
#include "keypress.h"
#include "libutil/libutil.h"

/**
 * Convert raw buffer index into a render buffer index.
 *
 * @param row
 * @param cursx
 * @return int
 */
static int
get_renderx (row_buffer_t* row, unsigned int cursx) {
  unsigned int renderx = 0;
  for (unsigned int i = 0; i < cursx; i++) {
    if (row->raw[i] == '\t') {
      renderx += (editor.config.tab_sz - 1) - (renderx % editor.config.tab_sz);
    }
    renderx++;
  }

  return renderx;
}

static void
window_scroll (void) {
  editor.renderx = 0;
  if (cursor_on_content_line()) {
    editor.renderx
      = get_renderx(&editor.buf.rows[editor.curs.y], editor.curs.x);
  }

  // Check if the cursor is above the visible window; if so, scroll up to it.
  if (cursor_above_visible_window()) {
    editor.curs.row = editor.curs.y;
  }

  // Check if the cursor is below the visible window and adjust
  if (cursor_below_visible_window()) {
    editor.curs.row = editor.curs.y - editor.win.rows + 1;
  }

  if (cursor_left_of_visible_window()) {
    editor.curs.col = editor.renderx;
  }

  if (cursor_right_of_visible_window()) {
    editor.curs.col = editor.renderx - editor.win.cols + 1;
  }
}

static void
window_draw_splash (buffer_t* buf) {
  char splash[80];
  int  splash_len
    = snprintf(splash, sizeof(splash), "Tabloid - version %s", TABLOID_VERSION);

  if (splash_len > editor.win.cols) {
    splash_len = editor.win.cols;
  }

  int padding = (editor.win.cols - splash_len) / 2;
  if (padding) {
    buffer_append(buf, editor.config.ln_prefix);
    padding--;
  }

  while (padding--) {
    buffer_append(buf, " ");
  }

  buffer_append_with(buf, splash, splash_len);
}

static void
window_draw_status_bar (buffer_t* buf) {
  buffer_append(buf, ESCAPE_SEQ_INVERT_COLOR);

  buffer_append(buf, editor.sbar.msg);

  unsigned int len = 0;
  while (len < editor.win.cols - strlen(editor.sbar.msg)) {
    buffer_append(buf, " ");
    len++;
  }

  buffer_append(buf, ESCAPE_SEQ_NORM_COLOR);
}

static void
window_draw_row (buffer_t* buf, row_buffer_t* row) {
  int len = row->renderbuf_sz - editor.curs.col;
  if (len < 0) {
    len = 0;
  }
  if (len > editor.win.cols) {
    len = editor.win.cols;
  }

  buffer_append_with(buf, &row->renderbuf[editor.curs.col], len);
}

static void
window_draw_rows (buffer_t* buf) {
  // For every row in the entire window...
  for (unsigned int y = 0; y < editor.win.rows; y++) {
    // Grab the visible row
    int visible_row_idx = y + editor.curs.row;
    // If the visible row index is > the number of buffered rows...
    if (visible_row_idx >= editor.buf.num_rows) {
      // If no content...
      if (editor.buf.num_rows == 0 && y == editor.win.rows / 3) {
        window_draw_splash(buf);
      } else {
        buffer_append(buf, editor.config.ln_prefix);
      }
    } else {
      // Has row content; render it...
      row_buffer_t current_row = editor.buf.rows[visible_row_idx];
      window_draw_row(buf, &current_row);
    }

    // Clear line to the right of the cursor
    buffer_append(buf, ESCAPE_SEQ_ERASE_LN_RIGHT_OF_CURSOR);
    buffer_append(buf, "\r\n");
  }
}

void
window_refresh (void) {
  window_scroll();

  buffer_t* buf = buffer_init(NULL);

  // Hide and later show the cursor to prevent flickering when drawing the grid
  buffer_append(buf, ESCAPE_SEQ_CURSOR_HIDE);
  buffer_append(buf, ESCAPE_SEQ_CURSOR_POS);

  window_draw_rows(buf);
  window_draw_status_bar(buf);

  cursor_set_position(buf);

  buffer_append(buf, ESCAPE_SEQ_CURSOR_SHOW);

  write(STDOUT_FILENO, buffer_state(buf), buffer_size(buf));
  buffer_free(buf);
}

void
window_clear (void) {
  write(STDOUT_FILENO, ESCAPE_SEQ_CLEAR_SCREEN, 4);
  write(STDOUT_FILENO, ESCAPE_SEQ_CURSOR_POS, 3);
}

void
window_set_status_bar_msg (const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(editor.sbar.msg, sizeof(editor.sbar.msg), fmt, ap);
  va_end(ap);
}
