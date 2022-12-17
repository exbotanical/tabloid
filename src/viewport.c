/**
 * @file viewport.c
 * @author goldmund
 * @brief Viewport rendering and cursor control module
 * @version 0.1
 * @date 2021-07-05
 *
 * @copyright Copyright (c) 2021 Matthew Zito (goldmund)
 *
 */

#include "viewport.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "../deps/libutil/buffer.h"
#include "render.h"

/***********
 * Cursor Ctrl
 ***********/

/**
 * @brief Move the cursor position
 * @param key
 * @todo allow custom mappings
 */
void cursor_mv(int key) {
  Row* row = (T.curs_y >= T.num_rows) ? NULL : &T.row[T.curs_y];

  switch (key) {
    case ARR_L:
      if (T.curs_x != 0) {
        T.curs_x--;
        // if not first line, we allow user to go <- at line begin,
        // snapping to the previous line
      } else if (T.curs_y > 0) {
        T.curs_y--;
        T.curs_x = T.row[T.curs_y].size;
      }
      break;
    case ARR_D:
      // allow cursor advance past bottom of viewport (but not past file)
      if (T.curs_y < T.num_rows) T.curs_y++;
      break;
    case ARR_U:
      if (T.curs_y != 0) T.curs_y--;
      break;
    case ARR_R:
      if (row && T.curs_x < row->size) {
        T.curs_x++;

        // if cursor not at EOF, -> snaps to next line
      } else if (row && T.curs_x == row->size) {
        T.curs_y++;
        T.curs_x = 0;
      }
      break;
  }

  // snap-to-row
  // prevent cursor from falling out of the viewport if moving vertically off of
  // a longer row and onto a shorter one

  // we have to set the row again, then curs_x to the end of the line if it is
  // to the right of the end of said line, where NULL is a line of len 0
  row = (T.curs_y >= T.num_rows) ? NULL : &T.row[T.curs_y];

  int rowlen = row ? row->size : 0;
  if (T.curs_x > rowlen) T.curs_x = rowlen;
}

/**
 * @brief Get the Cursor Pos object
 *
 * @param rows
 * @param cols
 * @return int
 *
 * @see https://vt100.net/docs/vt100-ug/chapter3.html#DSR
 * @see https://vt100.net/docs/vt100-ug/chapter3.html#CPR
 */
int cursor_get_pos(int* rows, int* cols) {
  char buf[32];
  unsigned int i = 0;

  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

  while (i < sizeof(buf) - 1) {
    if (NEQ_1(read(STDIN_FILENO, &buf[i], 1))) break;
    if (buf[i] == 'R') break;

    i++;
  }

  buf[i] = NULL_TERMINATOR;

  // ensure response is an esc sequence
  if (buf[0] != ESCAPE || buf[1] != '[') return -1;
  // pull ints out of the response esc seq - these are our rows, cols
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;

  return 0;
}

/**
 * @brief Convert a 'chars' index (`curs_x`) into a render buffer index
 * (`render_x`)
 *
 * @return int
 */
int cidx_to_ridx(Row* row, int cx) {
  int rx = 0;
  int j;

  for (j = 0; j < cx; j++) {
    if (row->chars[j] == '\t') {
      // how many cols are to the right of the tab stop?
      // use to determine cols to left of the next
      rx += (TAB_SIZE - 1) - (rx % TAB_SIZE);
    }
    // to next tab stop
    rx++;
  }

  return rx;
}

/**
 * @brief Convert a render buffer index (`render_x`) into a 'chars' index
 * (`curs_x`)
 *
 * @return int
 */
int ridx_to_cidx(Row* row, int rx) {
  int cx = 0;
  int j;

  // much akin to `cidx_to_ridx`, we iterate the chars,
  // stopping when cx == rx
  for (j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t') {
      cx += (TAB_SIZE - 1) - (cx % TAB_SIZE);
    }

    cx++;

    if (cx > rx) return cx;
  }

  // we shouldn't get here - this would be out of range
  return cx;
}

/***********
 * Viewport Ctrl
 ***********/

/**
 * @brief Clear the user's screen and set cursor position
 *
 * Performs cleanup by clearing the user's screen, setting the cursor position.
 * Utilized in lieu of `atexit` given this would also clear error messages
 * produced by `panic`
 *
 * @see https://vt100.net/docs/vt100-ug/chapter3.html#ED
 * @see https://vt100.net/docs/vt100-ug/chapter3.html#CUP
 * @todo use ncurses for better term support
 */
void clear_screen(void) {
  scroll();

  Buffer* buffer = buffer_init(NULL);

  // mitigate cursor flash on repaint - hide / show
  buffer_append(buffer, "\x1b[?25l");
  // TODO use terminfo db
  buffer_append(buffer, "\x1b[H");

  draw_rows(buffer);
  draw_status_bar(buffer);
  draw_msg_bar(buffer);

  // cursor
  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (T.curs_y - T.row_offset) + 1,
           (T.render_x - T.col_offset) + 1);

  buffer_append(buffer, buf);

  buffer_append(buffer, "\x1b[?25h");

  write(STDOUT_FILENO, buffer->state, buffer->len);
  buffer_free(buffer);
}

/**
 * @brief Get the Window Size object
 *
 * @param cols
 * @return int return code
 *
 * @see http://www.delorie.com/djgpp/doc/libc/libc_495.html
 * @see https://vt100.net/docs/vt100-ug/chapter3.html#CUD
 */
int get_window_size(int* rows, int* cols) {
  struct winsize ws;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    // fallback if `ioctl` fails
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B" /* cursor fwd, cursor dwn */,
              12) != 12) {
      return -1;
    }

    return cursor_get_pos(rows, cols);
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

/**
 * @brief Control the cursor position in the Cartesian planar viewport
 */
void scroll(void) {
  T.render_x = 0;

  if (T.curs_y < T.num_rows) {
    T.render_x = cidx_to_ridx(&T.row[T.curs_y], T.curs_x);
  }
  // if cursor above visible viewport, scroll to cursor
  if (T.curs_y < T.row_offset) {
    T.row_offset = T.curs_y;
  }
  // correct if cursor below visible viewport
  if (T.curs_y >= T.row_offset + T.screen_rows) {
    T.row_offset = T.curs_y - T.screen_rows + 1;
  }
  // horizontal, inverse of above
  // here, we track `render_x` to account for both rendered chars and rendered
  // cursor pos
  if (T.render_x < T.col_offset) {
    T.col_offset = T.render_x;
  }

  if (T.render_x >= T.col_offset + T.screen_cols) {
    T.col_offset = T.render_x - T.screen_cols + 1;
  }
}
