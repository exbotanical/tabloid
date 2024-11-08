#include "window.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "const.h"
#include "cursor.h"
#include "editor.h"
#include "globals.h"
#include "keypress.h"

unsigned int line_pad = 0;

static void
window_draw_status_bar (buffer_t* buf) {
  buffer_append(buf, ESC_SEQ_INVERT_COLOR);

  buffer_append(buf, editor.s_bar.msg);
  for (unsigned int len = 0; len < editor.win.cols - strlen(editor.s_bar.msg); len++) {
    buffer_append(buf, " ");
  }

  buffer_append(buf, ESC_SEQ_NORM_COLOR);
}

static void
window_draw_command_bar (buffer_t* buf) {
  buffer_append(buf, editor.c_bar.msg);
  for (unsigned int len = 0; len < editor.win.cols - strlen(editor.s_bar.msg); len++) {
    buffer_append(buf, " ");
  }
}

static void
window_draw_row (buffer_t* buf, line_info_t* row, unsigned int lineno, int select_start, int select_end, bool is_current) {
  int len = row->line_length - (cursor_get_col_off());
  if (len < 0) {
    len = 0;
  }

  if ((unsigned int)len > (editor.win.cols - (line_pad + 1))) {
    len = (editor.win.cols - (line_pad + 1));
  }

  // Adjust for rows that are longer than the current viewport
  if (select_start < cursor_get_col_off()) {
    select_start = cursor_get_col_off();
  }
  if (select_end >= (len + (int)cursor_get_col_off())) {
    select_end = len + cursor_get_col_off() - 1;
  }

  char line[row->line_length];
  line_buffer_get_line(editor.r, lineno, line);

  bool is_selected = select_end != -1 && cursor_is_select_active() && select_end >= select_start;

  // Start at &line[col_off], go for `len` chars
  for (unsigned int i = cursor_get_col_off(); i < cursor_get_col_off() + len; i++) {
    if (is_selected) {
      if (i == select_start) {
        buffer_append(buf, ESC_SEQ_BG_COLOR(218));
      }
    }

    char tmp[2];
    tmp[0] = line[i];
    tmp[1] = '\0';

    buffer_append(buf, tmp);

    if (is_selected) {
      if (i == select_end) {
        if (is_current) {
          buffer_append(buf, ESC_SEQ_BG_COLOR(238));
        } else {
          buffer_append(buf, ESC_SEQ_NORM_COLOR);
        }
      }
    }
  }
}

void
window_clear (void) {
  write(STDOUT_FILENO, ESC_SEQ_CLEAR_SCREEN, 4);
  write(STDOUT_FILENO, ESC_SEQ_CURSOR_POS, 3);
}

void
window_refresh (void) {
  window_scroll();

  buffer_t* buf = buffer_init(NULL);

  // Hide and later show the cursor to prevent flickering when drawing the grid
  buffer_append(buf, ESC_SEQ_CURSOR_HIDE);
  buffer_append(buf, ESC_SEQ_CURSOR_POS);

  window_draw_rows(buf);
  window_draw_status_bar(buf);
  window_draw_command_bar(buf);

  cursor_set_position(buf);

  buffer_append(buf, ESC_SEQ_CURSOR_SHOW);

  write(STDOUT_FILENO, buffer_state(buf), buffer_size(buf));
  buffer_free(buf);
}

void
window_scroll (void) {
  // Check if the cursor is above the visible window; if so, scroll up to it.
  if (cursor_above_visible_window()) {
    cursor_set_row_off(cursor_get_y());
  }

  // Check if the cursor is below the visible window and adjust
  if (cursor_below_visible_window()) {
    cursor_set_row_off(cursor_get_y() - editor.win.rows + 1);
  }

  if (cursor_left_of_visible_window()) {
    cursor_set_col_off(cursor_get_x());
  }

  if (cursor_right_of_visible_window()) {
    cursor_set_col_off(cursor_get_x() - (editor.win.cols - (line_pad + 1)) + 1);
  }
}

void
window_set_status_bar_msg (const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(editor.s_bar.msg, sizeof(editor.s_bar.msg), fmt, ap);
  va_end(ap);
}

void
window_set_command_bar_msg (const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(editor.c_bar.msg, sizeof(editor.c_bar.msg), fmt, ap);
  va_end(ap);
}

void
window_draw_rows (buffer_t* buf) {
  unsigned int lineno = cursor_get_row_off();
  line_pad            = log10(editor.r->num_lines) + 1;

  if (line_pad < DEFAULT_LNPAD) {
    line_pad = DEFAULT_LNPAD;
  }

  // For every row in the entire window...
  for (unsigned int y = 0; y < editor.win.rows; y++) {
    // Grab the visible row
    unsigned int visible_row_idx = y + cursor_get_row_off();
    // If the visible row index is > the number of buffered rows...
    if (visible_row_idx >= editor.r->num_lines) {
      buffer_append(buf, editor.conf.ln_prefix);
    } else {
      bool  is_current_line = visible_row_idx == cursor_get_y();
      char* lineno_str      = s_fmt("%*ld ", line_pad, ++lineno);

      // Highlighted lineno
      if (is_current_line) {
        buffer_append(buf, ESC_SEQ_COLOR(3));
      }
      buffer_append(buf, lineno_str);

      // Highlighted line
      if (is_current_line) {
        buffer_append(buf, ESC_SEQ_NORM_COLOR);
      }

      free(lineno_str);

      // Has row content; render it...
      line_info_t* current_row = (line_info_t*)array_get(editor.r->line_info, visible_row_idx);

      // Highlight the current row where the cursor is
      if (is_current_line) {
        buffer_append(buf, ESC_SEQ_BG_COLOR(238));
      }

      // TODO: refactor
      int select_start = -1;
      int select_end   = -1;

      if (cursor_is_select_active()) {
        bool is_ltr = cursor_is_select_ltr();

        if (is_ltr) {
          // If multiple lines are highlighted, and this line falls between the
          // anchor and offset lines, we just highlight the entire line
          if (y > cursor_get_anchor_y() && y < cursor_get_offset_y()) {
            select_start = 0;
            select_end   = (current_row->line_length) - 1;
          }

          // If the anchor and offset are on the same line,
          // just highlight between the anchor x and offset x
          if (y == cursor_get_anchor_y() && y == cursor_get_offset_y()) {
            select_start = cursor_get_anchor_x();
            select_end   = cursor_get_offset_x() - 1;
          }

          // If we're on the anchor line, we highlight from the anchor x onward
          else if (y == cursor_get_anchor_y()) {
            select_start = cursor_get_anchor_x();
            select_end   = current_row->line_length - 1;
          }

          // If we're on the offset line, we highlight until the the anchor x
          else if (y == cursor_get_offset_y()) {
            select_start = 0;
            select_end   = cursor_get_offset_x() - 1;
          }
        } else {
          if (y < cursor_get_anchor_y() && y > cursor_get_offset_y()) {
            select_start = 0;
            select_end   = current_row->line_length - 1;
          }

          if (y == cursor_get_anchor_y() && y == cursor_get_offset_y()) {
            select_start = cursor_get_offset_x();
            select_end   = cursor_get_anchor_x() - 1;
          } else if (y == cursor_get_anchor_y()) {
            select_start = 0;
            select_end   = cursor_get_anchor_x() - 1;
          } else if (y == cursor_get_offset_y()) {
            select_start = cursor_get_offset_x();
            select_end   = current_row->line_length - 1;
          }
        }
      }

      window_draw_row(buf, current_row, visible_row_idx, select_start, select_end, is_current_line);

      if (is_current_line) {
        // If it's the current row, reset the highlight after drawing the row
        int padding_len = (editor.win.cols + cursor_get_col_off()) - (current_row->line_length + line_pad + 1);
        if (padding_len > 0) {
          for (int i = 0; i < padding_len; i++) {
            buffer_append(buf, " ");  // Highlight entire row till the end
          }
        }
        buffer_append(buf, ESC_SEQ_NORM_COLOR);
      }
    }

    // Clear line to the right of the cursor
    buffer_append(buf, ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR);
    buffer_append(buf, CRLF);
  }
}
