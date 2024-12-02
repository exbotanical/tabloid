#include "window.h"

#include <libgen.h>  // TODO: compat?
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "command_bar.h"
#include "const.h"
#include "cursor.h"
#include "globals.h"
#include "keypress.h"
#include "line_buffer.h"
#include "line_editor.h"
#include "status_bar.h"

unsigned int line_pad = 0;

void
window_draw_status_bar (buffer_t* buf) {
  // TODO: Cleanup
  bool has_file         = !!editor.filepath;
  bool is_dirty         = line_buffer_dirty(editor.line_ed.r);

  unsigned int num_cols = window_get_num_cols();
  unsigned int lineno   = editor.line_ed.curs.y + 1;
  unsigned int colno    = editor.line_ed.curs.x + 1;

  char* mode_str;

  switch (editor.mode) {
    case EDIT_MODE: {
      mode_str = "EDIT";
      break;
    }

    case COMMAND_MODE: {
      mode_str = "COMMAND";
      break;
    }
  }

  char* dirty_modifier = is_dirty ? "*" : "";

  char* file_info;
  if (has_file) {
    char* filepath_cp = s_copy(editor.filepath);
    file_info         = s_fmt(" | %s | %s%s", mode_str, basename(filepath_cp), dirty_modifier);
    free(filepath_cp);
  } else {
    file_info = s_fmt(" | %s | [%s%s]", mode_str, "No Name", dirty_modifier);
  }

  status_bar_set_left_component_msg(file_info);

  char* curs_info = s_fmt("| Ln %d, Col %d ", lineno, colno);
  status_bar_set_right_component_msg(curs_info);

  buffer_append(buf, ESC_SEQ_INVERT_COLOR);

  unsigned int component_len = strlen(editor.s_bar.left_component) + strlen(editor.s_bar.right_component);

  buffer_append(buf, editor.s_bar.left_component);
  for (unsigned int len = 0; len < num_cols - component_len; len++) {
    buffer_append(buf, " ");
  }
  buffer_append(buf, editor.s_bar.right_component);

  buffer_append(buf, ESC_SEQ_NORM_COLOR);
  free(file_info);
  free(curs_info);
}

void
window_draw_command_bar (buffer_t* buf) {
  if (editor.mode == COMMAND_MODE) {
    unsigned int num_cols = window_get_num_cols();
    if (editor.cmode == CB_MESSAGE) {
      buffer_append(buf, editor.cbar_msg);
      for (unsigned int len = 0; len < num_cols - strlen(editor.cbar_msg); len++) {
        buffer_append(buf, " ");
      }
      buffer_append(buf, ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR);
      return;
    }

    buffer_append(buf, COMMAND_BAR_PREFIX);

    line_info_t* row = (line_info_t*)array_get(editor.c_bar.r->line_info, 0);
    if (!row) {
      buffer_append(buf, " ");
      buffer_append(buf, ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR);
      return;
    }

    int len = row->line_length - (cursor_get_col_off(&editor.c_bar));
    if (len < 0) {
      len = 0;
    }

    if ((unsigned int)len > (num_cols - 1)) {
      len = (num_cols - 1);
    }

    char line[row->line_length];
    line_buffer_get_line(editor.c_bar.r, 0, line);

    for (unsigned int i = cursor_get_col_off(&editor.c_bar); i < cursor_get_col_off(&editor.c_bar) + len; i++) {
      buffer_append_char(buf, line[i]);
    }

    if (len == 0) {
      buffer_append(buf, " ");
    }
  } else {
    buffer_append(buf, " ");
  }

  buffer_append(buf, ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR);
}

static void
window_compute_select_range (unsigned int row_num, line_info_t* current_line, int* start_ptr, int* end_ptr) {
  if (cursor_is_select_active(&editor.line_ed)) {
    bool is_ltr = cursor_is_select_ltr(&editor.line_ed);

    if (is_ltr) {
      // If multiple lines are highlighted, and this line falls between the
      // anchor and offset lines, we just highlight the entire line
      if (row_num > cursor_get_anchor_y(&editor.line_ed) && row_num < cursor_get_offset_y(&editor.line_ed)) {
        *start_ptr = 0;
        *end_ptr   = (current_line->line_length) - 1;
      }

      // If the anchor and offset are on the same line,
      // just highlight between the anchor x and offset x
      if (row_num == cursor_get_anchor_y(&editor.line_ed) && row_num == cursor_get_offset_y(&editor.line_ed)) {
        *start_ptr = cursor_get_anchor_x(&editor.line_ed);
        *end_ptr   = cursor_get_offset_x(&editor.line_ed) - 1;
      }

      // If we're on the anchor line, we highlight from the anchor x onward
      else if (row_num == cursor_get_anchor_y(&editor.line_ed)) {
        *start_ptr = cursor_get_anchor_x(&editor.line_ed);
        *end_ptr   = current_line->line_length - 1;
      }

      // If we're on the offset line, we highlight until the the anchor x
      else if (row_num == cursor_get_offset_y(&editor.line_ed)) {
        *start_ptr = 0;
        *end_ptr   = cursor_get_offset_x(&editor.line_ed) - 1;
      }
    } else {
      if (row_num < cursor_get_anchor_y(&editor.line_ed) && row_num > cursor_get_offset_y(&editor.line_ed)) {
        *start_ptr = 0;
        *end_ptr   = current_line->line_length - 1;
      }

      if (row_num == cursor_get_anchor_y(&editor.line_ed) && row_num == cursor_get_offset_y(&editor.line_ed)) {
        *start_ptr = cursor_get_offset_x(&editor.line_ed);
        *end_ptr   = cursor_get_anchor_x(&editor.line_ed) - 1;
      } else if (row_num == cursor_get_anchor_y(&editor.line_ed)) {
        *start_ptr = 0;
        *end_ptr   = cursor_get_anchor_x(&editor.line_ed) - 1;
      } else if (row_num == cursor_get_offset_y(&editor.line_ed)) {
        *start_ptr = cursor_get_offset_x(&editor.line_ed);
        *end_ptr   = current_line->line_length - 1;
      }
    }
  }
}

static void
window_draw_row (buffer_t* buf, line_info_t* row, unsigned int lineno, int select_start, int select_end, bool is_current) {
  int len = row->line_length - (cursor_get_col_off(&editor.line_ed));
  if (len < 0) {
    len = 0;
  }

  if ((unsigned int)len > (window_get_num_cols() - (line_pad + 1))) {
    len = (window_get_num_cols() - (line_pad + 1));
  }

  // Adjust for rows that are longer than the current viewport
  if (select_start < cursor_get_col_off(&editor.line_ed)) {
    select_start = cursor_get_col_off(&editor.line_ed);
  }
  if (select_end >= (len + (int)cursor_get_col_off(&editor.line_ed))) {
    select_end = len + cursor_get_col_off(&editor.line_ed) - 1;
  }

  char line[row->line_length];
  line_buffer_get_line(editor.line_ed.r, lineno, line);

  bool is_selected = select_end != -1 && cursor_is_select_active(&editor.line_ed) && select_end >= select_start;

  // Start at &line[col_off], go for `len` chars
  for (unsigned int i = cursor_get_col_off(&editor.line_ed); i < cursor_get_col_off(&editor.line_ed) + len; i++) {
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

extern inline unsigned int
window_get_num_rows (void) {
  return editor.win.rows;
}

extern inline unsigned int
window_get_num_cols (void) {
  return editor.win.cols;
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

  switch (editor.mode) {
    case EDIT_MODE: {
      cursor_set_position(&editor.line_ed, buf);
      break;
    }

    case COMMAND_MODE: {
      if (editor.cmode == CB_MESSAGE) {
        cursor_set_position(&editor.line_ed, buf);
        break;
      }
      cursor_set_position_command_bar(&editor.c_bar, buf);
      break;
    }
  }

  buffer_append(buf, ESC_SEQ_CURSOR_SHOW);

  write(STDOUT_FILENO, buffer_state(buf), buffer_size(buf));
  buffer_free(buf);
}

void
window_scroll (void) {
  // Check if the cursor is above the visible window; if so, scroll up to it.
  if (cursor_above_visible_window(&editor.line_ed)) {
    cursor_set_row_off(&editor.line_ed, cursor_get_y(&editor.line_ed));
  }

  // Check if the cursor is below the visible window and adjust
  if (cursor_below_visible_window(&editor.line_ed)) {
    cursor_set_row_off(&editor.line_ed, cursor_get_y(&editor.line_ed) - window_get_num_rows() + 1);
  }

  if (cursor_left_of_visible_window(&editor.line_ed)) {
    cursor_set_col_off(&editor.line_ed, cursor_get_x(&editor.line_ed));
  }

  if (cursor_right_of_visible_window(&editor.line_ed)) {
    cursor_set_col_off(&editor.line_ed, cursor_get_x(&editor.line_ed) - (window_get_num_cols() - (line_pad + 1)) + 1);
  }
}

void
window_draw_rows (buffer_t* buf) {
  unsigned int lineno = cursor_get_row_off(&editor.line_ed);
  line_pad            = log10(editor.line_ed.r->num_lines) + 1;

  if (line_pad < DEFAULT_LNPAD) {
    line_pad = DEFAULT_LNPAD;
  }

  // For every row in the entire window...
  for (unsigned int y = 0; y < window_get_num_rows(); y++) {
    // Grab the visible row
    unsigned int visible_row_idx = y + cursor_get_row_off(&editor.line_ed);
    // If the visible row index is > the number of buffered rows...
    if (visible_row_idx >= editor.line_ed.r->num_lines) {
      buffer_append(buf, editor.conf.ln_prefix);
    } else {
      bool  is_current_line = visible_row_idx == cursor_get_y(&editor.line_ed);
      char* lineno_str      = s_fmt("%*ld ", line_pad, ++lineno);

      // Highlighted lineno
      if (is_current_line) {
        buffer_append(buf, ESC_SEQ_COLOR(3));
      }

      buffer_append(buf, lineno_str);
      free(lineno_str);

      // Highlight the current row where the cursor is
      if (is_current_line) {
        buffer_append(buf, ESC_SEQ_NORM_COLOR);
        buffer_append(buf, ESC_SEQ_BG_COLOR(238));
      }

      // Has row content; render it...
      line_info_t* current_row = (line_info_t*)array_get(editor.line_ed.r->line_info, visible_row_idx);

      // TODO: refactor
      int select_start = -1;
      int select_end   = -1;

      if (cursor_is_select_active(&editor.line_ed)) {
        bool is_ltr = cursor_is_select_ltr(&editor.line_ed);

        if (is_ltr) {
          // If multiple lines are highlighted, and this line falls between the
          // anchor and offset lines, we just highlight the entire line
          if (y > cursor_get_anchor_y(&editor.line_ed) && y < cursor_get_offset_y(&editor.line_ed)) {
            select_start = 0;
            select_end   = (current_row->line_length) - 1;
          }

          // If the anchor and offset are on the same line,
          // just highlight between the anchor x and offset x
          if (y == cursor_get_anchor_y(&editor.line_ed) && y == cursor_get_offset_y(&editor.line_ed)) {
            select_start = cursor_get_anchor_x(&editor.line_ed);
            select_end   = cursor_get_offset_x(&editor.line_ed) - 1;
          }

          // If we're on the anchor line, we highlight from the anchor x onward
          else if (y == cursor_get_anchor_y(&editor.line_ed)) {
            select_start = cursor_get_anchor_x(&editor.line_ed);
            select_end   = current_row->line_length - 1;
          }

          // If we're on the offset line, we highlight until the the anchor x
          else if (y == cursor_get_offset_y(&editor.line_ed)) {
            select_start = 0;
            select_end   = cursor_get_offset_x(&editor.line_ed) - 1;
          }
        } else {
          if (y < cursor_get_anchor_y(&editor.line_ed) && y > cursor_get_offset_y(&editor.line_ed)) {
            select_start = 0;
            select_end   = current_row->line_length - 1;
          }

          if (y == cursor_get_anchor_y(&editor.line_ed) && y == cursor_get_offset_y(&editor.line_ed)) {
            select_start = cursor_get_offset_x(&editor.line_ed);
            select_end   = cursor_get_anchor_x(&editor.line_ed) - 1;
          } else if (y == cursor_get_anchor_y(&editor.line_ed)) {
            select_start = 0;
            select_end   = cursor_get_anchor_x(&editor.line_ed) - 1;
          } else if (y == cursor_get_offset_y(&editor.line_ed)) {
            select_start = cursor_get_offset_x(&editor.line_ed);
            select_end   = current_row->line_length - 1;
          }
        }
      }

      if (current_row) {
        window_draw_row(buf, current_row, visible_row_idx, select_start, select_end, is_current_line);
      }

      if (is_current_line) {
        int current_row_len = current_row ? current_row->line_length : 0;
        // If it's the current row, reset the highlight after drawing the row
        int padding_len = (window_get_num_cols() + cursor_get_col_off(&editor.line_ed)) - (current_row_len + line_pad + 1);
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
