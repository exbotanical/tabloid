#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include "tty.h"

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "cursor.h"
#include "editor.h"
#include "exception.h"
#include "globals.h"
#include "keypress.h"

void
tty_disable_raw_mode (void) {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &editor.tty.og_tty);
}

void
tty_enable_raw_mode (void) {
  int ret;
  if ((ret = tcgetattr(STDIN_FILENO, &editor.tty.og_tty)) != 0) {
    panic("Call to tcgetattr failed with return code %d\n", ret);
  }

  struct termios tty  = editor.tty.og_tty;

  tty.c_iflag        &= ~(
    // Don't translate carriage returns into newlines
    ICRNL
    // Resume transmission i.e. ignore legacy ctrl+s/ctrl+q pause/resume
    | IXON
    // Disable a legacy serial port break condition thingy thingy - TODO: what?
    // Do we really need this?
    | BRKINT
    // Don't expect parity bit - TODO: needed?
    | INPCK
    // Disable feature that makes 8th bit of each input set to 0 - TODO: needed?
    | ISTRIP
  );

  tty.c_oflag &= ~(
    // Deactivate post-processing output (e.g. inserting CR to LF)
    OPOST
  );

  tty.c_lflag &= ~(
    // Don't echo back input
    ECHO
    // Turn off canonical mode (e.g. read input per byte instead of per line)
    | ICANON
    // Turn off some of the additional input processing; we don't need it -
    // TODO: is this needed either?
    | IEXTEN
    // Ignore signals e.g. ctrl+c
    | ISIG
  );

  tty.c_cflag |= (
    // Set char size to 8-bits (standard)
    CS8
  );

  // Min num chars to read before `read` returns. If zero, `read` is
  // non-blocking and returns as soon as data is available.
  tty.c_cc[VMIN]  = 0;
  //  Specifies how long to wait for input before returning, in units of 0.1
  //  seconds. i.e. 1 = 100ms
  tty.c_cc[VTIME] = 1;

  if ((ret = tcsetattr(STDIN_FILENO, TCSAFLUSH, &tty)) != 0) {
    panic("Call to tcsetattr failed with return code %d\n", ret);
  }
}

int
tty_get_window_sz (unsigned int *rows, unsigned int *cols) {
  struct winsize ws;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    // Fallback method - we move the cursor to the bottom right corner and query
    // its position to get the x, y
    if (write(STDOUT_FILENO, ESC_SEQ_CURSOR_MAX_FWD ESC_SEQ_CURSOR_MAX_DWN, 12) != 12) {
      return -1;
    }
    return cursor_get_position(rows, cols);
  }

  *rows = ws.ws_row;
  *cols = ws.ws_col;

  return 0;
}

void
tty_clear (void) {
  write(STDOUT_FILENO, ESC_SEQ_CURSOR_POS ESC_SEQ_CLEAR_SCREEN ESC_SEQ_CLEAR_SCROLLBUF, 12);
}
