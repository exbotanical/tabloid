/**
 * @file render.c
 * @author goldmund
 * @brief Handles editor rendering and in-viewport drawing
 * @version 0.1
 * @date 2021-07-05
 *
 * @copyright Copyright (c) 2021 Matthew Zito (goldmund)
 *
 */

#include "render.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../deps/libutil/buffer.h"
#include "../deps/libutil/str.h"
#include "common.h"
#include "syntax.h"

/**
 * @brief Draws vim-like tilde-prepended rows on the screen
 *
 * @todo set to customizable with lineno
 */
void draw_rows(Buffer* buffer) {
  int line;

  for (line = 0; line < T.screen_rows; line++) {
    int filerow = line + T.row_offset;
    // is this part of the text buffer, or a row that exists after its end?
    if (filerow >= T.num_rows) {
      // no file was opened; blank editor
      if (T.num_rows == 0 && line == T.screen_rows / 3) {
        // write branding
        char branding[80];

        int brandingLen = snprintf(branding, sizeof(branding), "%s -- v%s",
                                   APP_NAME, APP_VERSION);

        // truncate?
        if (brandingLen > T.screen_cols) brandingLen = T.screen_cols;

        // padding
        int padding = (T.screen_cols - brandingLen) / 2;
        if (padding) {
          buffer_append(buffer, "~");
          padding--;
        }

        while (padding--) {
          buffer_append(buffer, " ");
        }
        buffer_append_with(buffer, branding, brandingLen);
      } else {
        buffer_append(buffer, "~");
      }
    } else {
      // subtract num of chars to left of the col offset from the row len
      int len = T.row[filerow].rsize - T.col_offset;

      if (len < 0) {
        len = 0;  // correct horizontal pos
      }
      if (len > T.screen_cols) {
        len = T.screen_cols;
      }

      /* syntax highlighting */
      char* c = &T.row[filerow].render[T.col_offset];
      // grab the assigned highlight / syntax type for given char
      unsigned char* hl = &T.row[filerow].highlight[T.col_offset];

      // track the current color so we don't have to
      // manually append a color seq before every single char
      int current_color = -1;
      int j;

      // for ea char
      for (j = 0; j < len; j++) {
        // handle non-printable characters
        if (iscntrl(c[j])) {
          char sym = (c[j] <= 26) ? '@' + c[j] : '?';

          buffer_append(buffer, "\x1b[7m");
          buffer_append(buffer, &sym);
          buffer_append(buffer, "\x1b[m");

          // revert to current color after inverting for non-printables
          if (current_color != -1) {
            char tmp_buffer[16];
            int c_len = snprintf(tmp_buffer, sizeof(tmp_buffer), "\x1b[%dm",
                                 current_color);

            buffer_append_with(buffer, tmp_buffer, c_len);
          }

        } else if (hl[j] == HL_DEFAULT) {
          if (current_color != -1) {
            buffer_append(buffer, "\x1b[39m");
            current_color = -1;
          }

          buffer_append_with(buffer, &c[j], 1);
        } else {
          int color = syntax_map_to_color(hl[j]);

          if (color != current_color) {
            current_color = color;

            char tmp_buffer[16];

            // write the esc sequence to a buffer
            int c_len =
                snprintf(tmp_buffer, sizeof(tmp_buffer), "\x1b[%dm", color);

            // append esc sequence to viewport text buffer
            buffer_append_with(buffer, tmp_buffer, c_len);
          }
          // append the actual character
          buffer_append_with(buffer, &c[j], 1);
        }
      }
      // reset color to default
      buffer_append(buffer, "\x1b[39m");
    }

    // clear line
    buffer_append(buffer, "\x1b[K");

    // mitigate missing line prefix on last line
    buffer_append(buffer, "\r\n");
  }
}

/**
 * @brief Render the message bar to the viewport
 *
 * Times out after user input or five seconds
 *
 * @param buffer
 */
void draw_msg_bar(Buffer* buffer) {
  // clear message bar
  buffer_append(buffer, "\x1b[K");

  int msglen = strlen(T.status_msg);

  // ensure it fits
  if (msglen > T.screen_cols) {
    msglen = T.screen_cols;
  }

  // only display message if it is less than 5s old
  if (msglen && time(NULL) - T.status_msg_time < 5) {
    buffer_append_with(buffer, T.status_msg, msglen);
  }
}

/**
 * @brief Render the status bar
 * @param buffer
 *
 * @see https://vt100.net/docs/vt100-ug/chapter3.html#SGR
 */
void draw_status_bar(Buffer* buffer) {
  // switch to inverted hues
  buffer_append(buffer, "\x1b[7m");

  char status[80], rstatus[80];

  int len = snprintf(status, sizeof(status), "%.20s - %d lines %s",
                     T.filename ? T.filename : "[No Name]", T.num_rows,
                     T.dirty ? "(modified)" : "");

  // lineno
  int rlen = snprintf(rstatus, sizeof(rstatus), "%s | %d/%d",
                      // syntax hl?
                      T.syntax ? T.syntax->f_type : "no file type detected",
                      // current line
                      T.curs_y + 1, T.num_rows);

  // truncate
  if (len > T.screen_cols) {
    len = T.screen_cols;
  }
  buffer_append_with(buffer, status, len);

  while (len < T.screen_cols) {
    // print spaces until we hit the second status str
    if (T.screen_cols - len == rlen) {
      buffer_append_with(buffer, rstatus, rlen);
      break;
    } else {
      buffer_append(buffer, " ");
      len++;
    }
  }

  // clear formatting
  buffer_append(buffer, "\x1b[m");
  // print NL after first status bar
  buffer_append(buffer, "\r\n");
}

/**
 * @brief Set the Status Message object
 *
 * @param fmt
 * @param ... variadic number of arguments to satisfy format string `fmt`
 */
void set_status_msg(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  // this calls `va_arg` for us
  vsnprintf(T.status_msg, sizeof(T.status_msg), fmt, ap);
  va_end(ap);

  T.status_msg_time = time(NULL);
}
