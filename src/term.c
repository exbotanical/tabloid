#include "term.h"

#include "editor.h"
#include "error.h"

#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#define CTRL_KEY(k) ((k) & 0x1f) /**< Mandate the ctrl binding that exits the program by setting upper 3 bits to 0 */

struct termios base_term; /**< Pointer ref for storing original, base termios configurations */

/**
 * @brief Enables raw mode and disables Canonical mode, allowing parsing of input byte-by-byte
 * 
 * Disables Canonical mode by modifying the current terminal attributes, 
 * retrieving the local mode bitmask and disabling `ECHO` bitflag, canonical mode,
 *  SIGINT, and SIGTSTP, etc
 */
void enableRawMode(void) {
  if (tcgetattr(STDIN_FILENO, &base_term) == -1) {
    panic("tcgetattr");
  }

  atexit(disableRawMode);
  
  struct termios raw = base_term;
  
  // conventions, C-m & datalink ctrl flow (C-s, C-q)
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  // disable carriage return on output (also -n)
  raw.c_oflag &= ~(OPOST);
  // set char size to 8 bits per byte
  raw.c_cflag |= (CS8);
  // modify local mode bitmask
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN /* C-v */ | ISIG);

  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
  
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

/**
 * @brief Disable raw mode by restoring termios attributes to defaults
 */
void disableRawMode(void) {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &base_term) == -1) {
    panic("tcsetattr");
  }
}

/**
 * @brief Process keypresses by mapping various ctrl keys et al to editor functionality
 * 
 */
void procKeyPress(void) {
  char c = readKey();

  switch (c) {
    case CTRL_KEY('q'):
      // clean
      clearScreen();
      exit(0);
      break;
  }
}
