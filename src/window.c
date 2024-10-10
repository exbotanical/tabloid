#include "window.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cursor.h"
#include "editor.h"
#include "globals.h"
#include "keypress.h"
#include "libutil/libutil.h"

unsigned int line_pad = 0;

/**
 * Convert raw buffer index into a render buffer index.
 *
 * @param row
 * @param cursx
 * @return int
 */
static int
get_render_x (line_buffer_t* row, unsigned int cursx) {
  unsigned int render_x = 0;
  for (unsigned int i = 0; i < cursx; i++) {
    if (row->raw[i] == '\t') {
      render_x += (editor.conf.tab_sz - 1) - (render_x % editor.conf.tab_sz);
    }
    render_x++;
  }

  return render_x;
}

static void
window_scroll (void) {
  // if (cursor_on_content_line()) {
  //   editor.curs.render_x
  //     = get_render_x(&editor.buf.lines[editor.curs.y], editor.curs.x);
  // }

  // Check if the cursor is above the visible window; if so, scroll up to it.
  if (cursor_above_visible_window()) {
    editor.curs.row_off = editor.curs.y;
  }

  // Check if the cursor is below the visible window and adjust
  if (cursor_below_visible_window()) {
    editor.curs.row_off = editor.curs.y - editor.win.rows + 1;
  }

  if (cursor_left_of_visible_window()) {
    editor.curs.col_off = editor.curs.x;
  }

  if (cursor_right_of_visible_window()) {
    editor.curs.col_off = editor.curs.x - (editor.win.cols - (line_pad + 1)) + 1;
  }
}

static void
window_draw_splash (buffer_t* buf) {
  char splash[80];
  int splash_len = snprintf(splash, sizeof(splash), "Tabloid - version %s", TABLOID_VERSION);

  if (splash_len > editor.win.cols) {
    splash_len = editor.win.cols;
  }

  int padding = (editor.win.cols - splash_len) / 2;
  if (padding) {
    buffer_append(buf, editor.conf.ln_prefix);
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

  buffer_append(buf, editor.s_bar.msg);
  for (unsigned int len = 0; len < editor.win.cols - strlen(editor.s_bar.msg); len++) {
    buffer_append(buf, " ");
  }

  buffer_append(buf, ESCAPE_SEQ_NORM_COLOR);
}

static void
window_draw_command_bar (buffer_t* buf) {
  buffer_append(buf, editor.c_bar.msg);
  for (unsigned int len = 0; len < editor.win.cols - strlen(editor.s_bar.msg); len++) {
    buffer_append(buf, " ");
  }
}

static void
window_draw_row (buffer_t* buf, line_buffer_t* row) {
  int len = row->render_buf_sz - (editor.curs.col_off);
  if (len < 0) {
    len = 0;
  }
  if (len > (editor.win.cols - (line_pad + 1))) {
    len = (editor.win.cols - (line_pad + 1));
  }

  buffer_append_with(buf, &row->render_buf[editor.curs.col_off], len);
}

static void
window_draw_rows (buffer_t* buf) {
  unsigned int lineno = 0;
  line_pad            = log10(editor.buf.num_lines) + 1;
  if (line_pad < DEFAULT_LNPAD) {
    line_pad = DEFAULT_LNPAD;
  }

  // For every row in the entire window...
  for (unsigned int y = 0; y < editor.win.rows; y++) {
    // Grab the visible row
    int visible_row_idx = y + editor.scroll.row_offset;
    // If the visible row index is > the number of buffered rows...
    if (visible_row_idx >= editor.buf.num_lines) {
      // If no content...
      if (editor.buf.num_lines == 0 && y == editor.win.rows / 3) {
        window_draw_splash(buf);
      } else {
        buffer_append(buf, editor.conf.ln_prefix);
      }
    } else {
      char* lineno_str = s_fmt("%*ld ", line_pad, ++lineno);

      // if (visible_row_idx == editor.curs.y) {
      //   buffer_append(buf, "\x1b[33m");
      // }
      buffer_append(buf, lineno_str);
      // if (visible_row_idx == editor.curs.y) {
      //   buffer_append(buf, ESCAPE_SEQ_NORM_COLOR);
      // }

      free(lineno_str);

      // Has row content; render it...
      line_buffer_t current_row = editor.buf.lines[visible_row_idx];

      // Highlight the current row where the cursor is
      // if (visible_row_idx == editor.curs.y) {
      //   buffer_append(buf, ESCAPE_SEQ_INVERT_COLOR);
      // }

      window_draw_row(buf, &current_row);

      // if (visible_row_idx == editor.curs.y) {
      //   // If it's the current row, reset the highlight after drawing the row
      //   int padding_len = editor.win.cols - (current_row.render_buf_sz +
      //   line_pad + 1); if (padding_len > 0) {
      //     for (int i = 0; i < padding_len; i++) {
      //       buffer_append(buf, " ");  // Highlight entire row till the end
      //     }
      //   }
      //   buffer_append(buf, ESCAPE_SEQ_NORM_COLOR);
      // }
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
  window_draw_command_bar(buf);

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
window_set_sbar_msg (const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(editor.s_bar.msg, sizeof(editor.s_bar.msg), fmt, ap);
  va_end(ap);
}

void
window_set_cbar_msg (const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(editor.c_bar.msg, sizeof(editor.c_bar.msg), fmt, ap);
  va_end(ap);
}
