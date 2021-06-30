#pragma GCC dependency "buffer.h"

#include "viewport.h"

#include "common.h"
#include "buffer.h"
#include "render.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>


#define E_BUFFER_INIT {NULL, 0} /**< Initialize an `extensible_buffer` */

/***********
 * Cursor Ctrl
 ***********/

/**
 * @brief Move the cursor position
 * @param key
 * @todo allow custom mappings
 */
void curs_mv(int key) {
  t_row* row = (T.curs_y >= T.numrows) ? NULL : &T.row[T.curs_y];

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
      if (T.curs_y < T.numrows) T.curs_y++;
      break;
    case ARR_U:
      if (T.curs_y != 0) T.curs_y--;
      break;
    case ARR_R:
      if (row &&T.curs_x < row->size) {
        T.curs_x++;

        // if cursor not at EOF, -> snaps to next line
      } else if (row && T.curs_x == row->size) {
        T.curs_y++;
        T.curs_x = 0;
      }
      break;
  }

  // snap-to-row
  // prevent cursor from falling out of the viewport if moving vertically off of a longer row
  // and onto a shorter one

  // we have to set the row again, then curs_x to the end of the line if it is to the right of the end
  // of said line, where NULL is a line of len 0
  row = (T.curs_y >= T.numrows) ? NULL : &T.row[T.curs_y];

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
int get_curs_pos(int* rows, int* cols) {
  char buf[32];
  unsigned int i = 0;

  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

  while (i < sizeof(buf) - 1) {
    if (NEQ_1(read(STDIN_FILENO, &buf[i], 1))) break;
    if (buf[i] == 'R') break;

    i++;
  }

  buf[i] = NULL_TERM;

  // ensure response is an esc sequence
  if (buf[0] != ESCAPE || buf[1] != '[') return -1;
  // pull ints out of the response esc seq - these are our rows, cols
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;

  return 0;
}

/**
 * @brief Convert a chars index (`curs_x`) into a render buffer index (`render_x`)
 *
 * @return int
 */
int curs_x_conv (t_row* row, int cx) {
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

/***********
 * Viewport Ctrl
 ***********/

/**
 * @brief Clear the user's screen and set cursor position
 *
 * Performs cleanup by clearing the user's screen, setting the cursor position.
 * Utilized in lieu of `atexit` given this would also clear error messages produced by `panic`
 *
 * @see https://vt100.net/docs/vt100-ug/chapter3.html#ED
 * @see https://vt100.net/docs/vt100-ug/chapter3.html#CUP
 * @todo use ncurses for better term support
 */
void clear_screen(void) {
  scroll();

  struct extensible_buffer e_buffer = E_BUFFER_INIT;

  // mitigate cursor flash on repaint - hide / show
  buf_extend(&e_buffer, "\x1b[?25l", 6);
  // TODO use terminfo db
  buf_extend(&e_buffer, "\x1b[H", 3);

  draw_rows(&e_buffer);
  draw_stats_bar(&e_buffer);
  draw_msg_bar(&e_buffer);

  // cursor
  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (T.curs_y - T.rowoff) + 1,
                                            (T.render_x - T.coloff) + 1);

  buf_extend(&e_buffer, buf, strlen(buf));

  buf_extend(&e_buffer, "\x1b[?25h", 6);

  write(STDOUT_FILENO, e_buffer.buf, e_buffer.len);
  free_e_buf(&e_buffer);
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
int get_win_sz(int* rows, int* cols) {
  struct winsize ws;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    // fallback if `ioctl` fails
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B" /* cursor fwd, cursor dwn */, 12) != 12) {
      return -1;
    }

    return get_curs_pos(rows, cols);
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

  if (T.curs_y < T.numrows) {
    T.render_x = curs_x_conv(&T.row[T.curs_y], T.curs_x);
  }
  // if cursor above visible viewport, scroll to cursor
  if (T.curs_y < T.rowoff) {
    T.rowoff = T.curs_y;
  }
  // correct if cursor below visible viewport
  if (T.curs_y >= T.rowoff + T.screenrows) {
    T.rowoff = T.curs_y - T.screenrows + 1;
  }
  // horizontal, inverse of above
  // here, we track `render_x` to account for both rendered chars and rendered cursor pos
  if (T.render_x < T.coloff) {
    T.coloff = T.render_x;
  }

  if (T.render_x >= T.coloff + T.screencols) {
    T.coloff = T.render_x - T.screencols + 1;
  }
}
