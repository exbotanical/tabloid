/**
 * @file editor.c
 * @author goldmund
 * @brief Core editor environment and input / rendering modes
 * @version 0.1
 * @date 2021-07-05
 *
 * @copyright Copyright (c) 2021 Matthew Zito (goldmund)
 *
 */

#include "editor.h"

#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "common.h"
#include "error.h"
#include "render.h"
#include "stream.h"
#include "viewport.h"

struct TtyConfig T;

/***********
 *
 * Modes
 *
 ***********/

/**
 * @brief Enables raw mode and disables Canonical mode, allowing parsing of
 * input byte-by-byte
 *
 * Disables Canonical mode by modifying the current terminal attributes,
 * retrieving the local mode bitmask and disabling `ECHO` bitflag, canonical
 * mode, SIGINT, and SIGTSTP, etc
 */
void enable_raw_mode(void) {
  // Get terminal attributes and store them in T
  if (tcgetattr(STDIN_FILENO, &T.og_tty) == -1) {
    panic("tcgetattr");
  }

  atexit(disable_raw_mode);

  struct termios raw = T.og_tty;

  // conventions, C-m & datalink ctrl flow (C-s, C-q)
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  // disable carriage return on output (also -n)
  raw.c_oflag &= ~(OPOST);
  // set char size to 8 bits per byte
  raw.c_cflag |= (CS8);
  // modify local mode bitmask
  raw.c_lflag &=
      ~(ECHO      // Echo input characters
        | ICANON  // Enable canonical mode
        | IEXTEN  // Enable implementation-defined input processing
        | ISIG    // When any of the characters INTR, QUIT, SUSP, or DSUSP are
                  // received, generate the corresponding signal);
      );
  raw.c_cc[VMIN] = 0;   // Minimum number of characters for non-canonical read
  raw.c_cc[VTIME] = 1;  // Timeout in deciseconds for non-canonical read

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
    panic("tcsetattr");
  }
}

/**
 * @brief Disable raw mode by restoring termios attributes to defaults
 */
void disable_raw_mode(void) {
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
void editor_init(void) {
  T.curs_x = 0;
  T.curs_y = 0;
  T.row_offset = 0;  // begin at top
  T.col_offset = 0;
  T.num_rows = 0;
  T.render_x = 0;
  T.row = NULL;
  T.filename = NULL;                  // will remain null if no file loaded
  T.status_msg[0] = NULL_TERMINATOR;  // default to no message at all
  T.status_msg_time = 0;
  T.dirty = 0;
  T.syntax = NULL;  // NULL == no file type detected

  if (get_window_size(&T.screen_rows, &T.screen_cols) == -1) {
    panic("get_window_size");
  }

  // row for status bar
  // prevent `draw_rows` from rendering a line at the bottom row
  T.screen_rows -= 2;
}
