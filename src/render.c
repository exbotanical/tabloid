#include "render.h"

#include "common.h"
#include "buffer.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/**
 * @brief Draws vim-like tilde-prepended rows on the screen
 *
 * @todo set to customizable with lineno
 */
void drawRows(struct appendBuf *abuf) {
  int line;

  for (line = 0; line < T.screenrows; line++) {
    int filerow = line + T.rowoff;
    // is this part of the text buffer, or a row that exists after its end?
    if (filerow >= T.numrows) {
      // no file was opened; blank editor
      if (T.numrows == 0 && line == T.screenrows / 3) {
        // write branding
        char branding[80];

        int brandingLen = snprintf(
          branding,
          sizeof(branding),
          "%s -- v%s", APPNAME, APPVERSION
        );

        // truncate?
        if (brandingLen > T.screencols) brandingLen = T.screencols;

        // padding
        int padding = (T.screencols - brandingLen) / 2;
        if (padding) {
          abufAppend(abuf, "~", 1);
          padding--;
        }

        while (padding--) abufAppend(abuf, " ", 1);
        abufAppend(abuf, branding, brandingLen);
      } else {
        abufAppend(abuf, "~", 1);
      }
    } else {
      // subtract num of chars to left of the col offset from the row len
      int len = T.row[filerow].rsize - T.coloff;

      if (len < 0) len = 0; // correct horizontal pos
      if (len > T.screencols) len = T.screencols;

      abufAppend(abuf, &T.row[filerow].render[T.coloff], len);
    }

    // clear line
    abufAppend(abuf, "\x1b[K", 3);

    // mitigate missing line prefix on last line
    abufAppend(abuf, "\r\n", 2);
  }
}

/**
 * @brief Render the message bar to the viewport
 *
 * Times out after user input or five seconds
 *
 * @param abuf
 */
void drawMessageBar(struct appendBuf *abuf) {
  // clear message bar
  abufAppend(abuf, "\x1b[K", 3);

  int msglen = strlen(T.statusmsg);

  // ensure it fits
  if (msglen > T.screencols) msglen = T.screencols;

  // only display message if it is less than 5s old
  if (msglen && time(NULL) - T.statusmsg_time < 5) {
    abufAppend(abuf, T.statusmsg, msglen);
  }
}

/**
 * @brief Render the status bar
 * @param abuf
 *
 * @see https://vt100.net/docs/vt100-ug/chapter3.html#SGR
 */
void drawStatusBar(struct appendBuf *abuf) {
  // switch to inverted hues
  abufAppend(abuf, "\x1b[7m", 4);

  char status[80], rstatus[80];

  int len = snprintf(
    status,
    sizeof(status),
    "%.20s - %d lines",
    T.filename ? T.filename : "[No Name]",
    T.numrows
  );

  // lineno
  int rlen = snprintf(
    rstatus,
    sizeof(rstatus),
    "%d/%d",
    // current line
    T.cursy + 1,
    T.numrows
  );

  // truncate
  if (len > T.screencols) len = T.screencols;
  abufAppend(abuf, status, len);

  while (len < T.screencols) {
    // print spaces until we hit the second status str
    if (T.screencols - len == rlen) {
      abufAppend(abuf, rstatus, rlen);
      break;
    } else {
      abufAppend(abuf, " ", 1);
      len++;
    }
  }

  // clear formatting
  abufAppend(abuf, "\x1b[m", 3);

  // print NL after first status bar
  abufAppend(abuf, "\r\n", 2);
}

/**
 * @brief Set the Status Message object
 *
 * @param fmt
 * @param ... variadic
 */
void setStatusMessage(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  // va_arg(&fmt);

  // this calls `va_arg` for us
  vsnprintf(
    T.statusmsg,
    sizeof(T.statusmsg),
    fmt,
    ap
  );

  va_end(ap);

  // passing `NULL` sets UNIX timestamp for now
  T.statusmsg_time = time(NULL);
}
