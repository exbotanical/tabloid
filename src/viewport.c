#include "viewport.h"

#include "common.h"
#include "buffer.h"
#include "render.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define ABUF_INIT {NULL, 0} /**< Initialize an `appendbuf` */
#define TAB_SIZE 8

/***********
 * Cursor Ctrl
 ***********/

/**
 * @brief Move the cursor position
 * @param key
 * @todo allow custom mappings
 */
void moveCursor(int key) {
  trow *row = (T.cursy >= T.numrows) ? NULL : &T.row[T.cursy];

  switch (key) {
    case ARR_L:
      if (T.cursx != 0) {
        T.cursx--;
        // if not first line, we allow user to go <- at line begin,
        // snapping to the previous line
      } else if (T.cursy > 0) {
        T.cursy--;
        T.cursx = T.row[T.cursy].size;
      }
      break;
    case ARR_D:
      // allow cursor advance past bottom of viewport (but not past file)
      if (T.cursy < T.numrows) T.cursy++;
      break;
    case ARR_U:
      if (T.cursy != 0) T.cursy--;
      break;
    case ARR_R:
      if (row &&T.cursx < row->size) {
        T.cursx++;

        // if cursor not at EOF, -> snaps to next line
      } else if (row && T.cursx == row->size) {
        T.cursy++;
        T.cursx = 0;
      }
      break;
  }

  // snap-to-row
  // prevent cursor from falling out of the viewport if moving vertically off of a longer row
  // and onto a shorter one

  // we have to set the row again, then cursx to the end of the line if it is to the right of the end
  // of said line, where NULL is a line of len 0
  row = (T.cursy >= T.numrows) ? NULL : &T.row[T.cursy];

  int rowlen = row ? row->size : 0;
  if (T.cursx > rowlen) T.cursx = rowlen;
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
int getCursorPos(int *rows, int *cols) {
  char buf[32];
  unsigned int i = 0;

  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
    if (buf[i] == 'R') break;

    i++;
  }

  buf[i] = '\0';

  // ensure response is an esc sequence
  if (buf[0] != ESCAPE || buf[1] != '[') return -1;
  // pull ints out of the response esc seq - these are our rows, cols
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;

  return 0;
}

/**
 * @brief Convert a chars index (`cursx`) into a render buffer index (`renderx`)
 *
 * @return int
 */
int cursxConv (trow *row, int cx) {
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
void clearScreen(void) {
  scroll();

  struct appendBuf abuf = ABUF_INIT;

  // mitigate cursor flash on repaint - hide / show
  abufAppend(&abuf, "\x1b[?25l", 6);
  // TODO use terminfo db
  abufAppend(&abuf, "\x1b[H", 3);

  drawRows(&abuf);
  drawStatusBar(&abuf);
  drawMessageBar(&abuf);

  // cursor
  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (T.cursy - T.rowoff) + 1,
                                            (T.renderx - T.coloff) + 1);

  abufAppend(&abuf, buf, strlen(buf));

  abufAppend(&abuf, "\x1b[?25h", 6);

  write(STDOUT_FILENO, abuf.buf, abuf.len);
  abufFree(&abuf);
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
int getWindowSize(int *rows, int *cols) {
  struct winsize ws;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    // fallback if `ioctl` fails
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B" /* cursor fwd, cursor dwn */, 12) != 12) {
      return -1;
    }

    return getCursorPos(rows, cols);
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
  T.renderx = 0;

  if (T.cursy < T.numrows) {
    T.renderx = cursxConv(&T.row[T.cursy], T.cursx);
  }
  // if cursor above visible viewport, scroll to cursor
  if (T.cursy < T.rowoff) {
    T.rowoff = T.cursy;
  }
  // correct if cursor below visible viewport
  if (T.cursy >= T.rowoff + T.screenrows) {
    T.rowoff = T.cursy - T.screenrows + 1;
  }
  // horizontal, inverse of above
  // here, we track `renderx` to account for both rendered chars and rendered cursor pos
  if (T.renderx < T.coloff) {
    T.coloff = T.renderx;
  }

  if (T.renderx >= T.coloff + T.screencols) {
    T.coloff = T.renderx - T.screencols + 1;
  }
}

/***********
 * Viewport Buffers
 ***********/

/**
 * @brief Allocate space for a new row, copy a given string at the end of the row array
 *
 * @param s
 * @param len
 */
void appendRow(char *s, size_t len) {
  // num bytes `trow` takes * the num of desired rows
  T.row = realloc(T.row, sizeof(trow) * (T.numrows + 1));

  // idx of new row to init
  int at = T.numrows;

  T.row[at].size = len;
  T.row[at].chars = malloc(len + 1);

  memcpy(T.row[at].chars, s, len);
  T.row[at].chars[len] = '\0';

  // init tabs
  T.row[at].rsize = 0;
  T.row[at].render = NULL;
  updateRow(&T.row[at]);

  T.numrows++;
}

/**
 * @brief USes chars str of a `trow` to fill the contents of the render string buffer
 *
 * @param row
 */
void updateRow(trow *row) {
  int tabs = 0;
  int j;

  // iterate chars of the row, counting tabs so as to alloc sufficient mem for `render`
  // max num chars needed for tabs is 8
  for (j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t') tabs++;
  }

  free(row->render);

  // row->size counts 1 for ea tab as it is, so we multiply by 7
  // and add to row->size to get max amt of mem needed for the rendered row
  row->render = malloc(row->size + tabs * (TAB_SIZE - 1) + 1);

  int idx = 0; // contains num of chars copied into row->render

  // after alloc, we check whether the current char is a tab - if it is, we append 1 space
  // as ea tab must advanced the cursor forward 1 col
  // we then append spaces until we reach a tab stop i.e. a col divisible by 8
  for (j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t') {
      row->render[idx++] = ' ';
      while (idx % TAB_SIZE != 0) row->render[idx++] = ' ';
    } else {
      row->render[idx++] = row->chars[j];
    }
  }

  row->render[idx] = '\0';
  row->rsize = idx;
}
