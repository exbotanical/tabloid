#include "editor.h"

#include "common.h"
#include "buffer.h"
#include "error.h"
#include "render.h"
#include "stream.h"
#include "viewport.h"

#include <errno.h>
// #include <stdio.h>
#include <stdlib.h>
// #include <string.h>
#include <termios.h>
#include <unistd.h>

struct ttyConfig T;

/***********
 *
 * Modes
 *
 ***********/

/**
 * @brief Enables raw mode and disables Canonical mode, allowing parsing of input byte-by-byte
 *
 * Disables Canonical mode by modifying the current terminal attributes,
 * retrieving the local mode bitmask and disabling `ECHO` bitflag, canonical mode,
 * SIGINT, and SIGTSTP, etc
 */
void enableRawMode(void) {
  if (tcgetattr(STDIN_FILENO, &T.og_tty) == -1) {
    panic("tcgetattr");
  }

  atexit(disableRawMode);

  struct termios raw = T.og_tty;

  // conventions, C-m & datalink ctrl flow (C-s, C-q)
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  // disable carriage return on output (also -n)
  raw.c_oflag &= ~(OPOST);
  // set char size to 8 bits per byte
  raw.c_cflag |= (CS8);
  // modify local mode bitmask
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
    panic("tcsetattr");
  }
}

/**
 * @brief Disable raw mode by restoring termios attributes to defaults
 */
void disableRawMode(void) {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &T.og_tty) == -1) {
    panic("tcsetattr");
  }
}

/***********
 *
 * Editor Ctrl
 *
 ***********/

/**
 * @brief Initialize the editor viewport
 */
void initEditor(void) {
  T.cursx = 0;
  T.cursy = 0;
  T.rowoff = 0; // begin at top
  T.coloff = 0;
  T.numrows = 0;
  T.renderx = 0;
  T.row = NULL;
  T.filename = NULL; // will remain null if no file loaded - what we want
  T.statusmsg[0] = '\0'; // default to no message at all
  T.statusmsg_time = 0;
	T.dirty = 0;

  if (getWindowSize(&T.screenrows, &T.screencols) == -1) {
    panic("getWindowSize");
  }

  // row for status bar
  // prevent `drawRows` from rendering a line at the bottom row
  T.screenrows -= 2;
}
