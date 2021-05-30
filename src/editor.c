/* Feature Test Macros */
#define _DEFAULT_SOURCE // ea. handles `getline` resolution
#define _BSD_SOURCE
#define _GNU_SOURCE

#include "editor.h"

#include "common.h"
#include "buffer.h"
#include "error.h"
#include "render.h"
#include "viewport.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

// `strdup` is not yet a part of Standard C
// it is standardized in POSIX.1-2008, and may or may not be provided by <string.h>
#ifndef STRDUP_H
#define STRDUP_H

char *strdup(const char *s) {
  size_t size = strlen(s) + 1;
  char *p = malloc(size);
  if (p != NULL) {
    memcpy(p, s, size);
  }
  return p;
}

#endif

struct ttyConfig T;

/***********
 * Modes
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
 * Editor Ctrl
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

  if (getWindowSize(&T.screenrows, &T.screencols) == -1) {
    panic("getWindowSize");
  }

  // row for status bar
  // prevent `drawRows` from rendering a line at the bottom row
  T.screenrows -= 2;
}

/***********
 * File I/O
 ***********/

/**
 * @brief Open a file in the editor
 *
 * @param filename
 */
void editorOpen(char *filename) {
  free(T.filename);
  T.filename = strdup(filename);

  FILE *fp = fopen(filename, "r");
  if (!fp) panic("fopen");

  char *line = NULL;
  size_t linecap = 0; /**< Tracks how much memory has been allocated */
  ssize_t linelen;

  // -1 at EOF
  while ((linelen = getline(&line, &linecap, fp)) != -1) {
    // we know ea. `trow` represents a single line of text, thus there is
    // no reason to store the newline, carriage return
    while (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r')) {
      linelen--;
    }

    appendRow(line, linelen);
  }

  free(line);
  fclose(fp);
}
