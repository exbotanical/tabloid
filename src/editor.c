#include "editor.h"

#include "common.h"
#include "error.h"
#include "buffer.h"

#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>

#define CTRL_KEY(k) ((k) & 0x1f) /**< Mandate the ctrl binding that exits the program by setting upper 3 bits to 0 */
#define ABUF_INIT {NULL, 0}; /**< Initialize an `appendbuf` */

struct ttyConfig {
  struct termios og_tty; /**< Pointer ref for storing original, original termios configurations */
  int screenrows;
  int screencols;
  int cursx, cursy; /**< User's cursor position coordinates */
};

struct ttyConfig T;

enum keybindings {
  ARR_U = 1000,
  ARR_D,
  ARR_R,
  ARR_L,
  DEL,
  HOME,
  END,
  PG_U,
  PG_D
};

/**
 * @brief Move the cursor position
 * @param key 
 * @todo allow custom mappings
 */
void moveCursor(int key) {
  switch (key) {
    case ARR_L:
      if (T.cursx != 0) T.cursx--;
      break;
    case ARR_D:
      if (T.cursy != T.screencols - 1) T.cursy++;
      break;
    case ARR_U:
      if (T.cursy != 0) T.cursy--;
      break;
    case ARR_R:
      if (T.cursy != T.screencols - 1) T.cursx++;
      break;
  }
}

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
  struct appendBuf abuf = ABUF_INIT;

  // mitigate cursor flash on repaint - hide / show
  abufAppend(&abuf, "\x1b[?25l", 6);
  // TODO use terminfo db
  abufAppend(&abuf, "\x1b[H", 3);
  
  drawRows(&abuf);

  // cursor
  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", T.cursy + 1, T.cursx + 1);
  abufAppend(&abuf, buf, strlen(buf));

  abufAppend(&abuf, "\x1b[?25h", 6);

  write(STDERR_FILENO, abuf.buf, abuf.len);
  abufFree(&abuf);
}

/**
 * @brief Draws vim-like tilde-prepended rows on the screen
 * @todo set to customizable with lineno
 */
void drawRows(struct appendBuf *abuf) {
  int line;

  for (line = 0; line < T.screenrows; line++) {
    // write branding
    if (line == T.screenrows / 3) {
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

    // clear line
    abufAppend(abuf, "\x1b[K", 3);

    // mitigate missing line prefix on last line
    if (line < T.screenrows - 1) {
      abufAppend(abuf, "\r\n", 2);
    }
  }
}

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

/**
 * @brief Wait for keypress from stdin and passthrough
 * @return int returned, keypress enum or escape sequence
 */
int readKey(void) {
  int nread;
  char c;

  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    // ignore EAGAIN as cygwin returns -1 here, because...Windows
    if (nread == -1 && errno != EAGAIN) {
      panic("read");
    }
  }

  // read the given escape sequence
  if (c == ESCAPE) {
    char seq[3];

    if (read(STDIN_FILENO, &seq[0], 1) != 1) return ESCAPE;
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return ESCAPE;

    if (seq[0] == '[') {
      if (seq[1] >= 0 && seq[1] <= '9') {
        if (read(STDIN_FILENO, &seq[2], 1) != 1) return ESCAPE;
        if (seq[2] == '~') {
          switch(seq[1]) {
            case '1': return HOME;
            case '3': return DEL;
            case '4': return END;
            case '5': return PG_U;
            case '6': return PG_D;
            case '7': return HOME;
            case '8': return END;
          }
        }
      } else {
        switch (seq[1]) {
          case 'A': return ARR_U;
          case 'B': return ARR_D;
          case 'C': return ARR_R;
          case 'D': return ARR_L;
          case 'H': return HOME;
          case 'F': return END;
        }
      }
    } else if (seq[0] == 'O') {
      switch (seq[1]) {
        case 'H': return HOME;
        case 'F': return END;
      }
    }

    return ESCAPE;
  } else {
    return c;
  }
}

/**
 * @brief Process keypresses by mapping various ctrl keys et al to tty functionality
 */
void procKeypress(void) {
  int c = readKey();

  switch (c) {
    case CTRL_KEY('q'):
      // clean
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);

      exit(0);
      break;
    
    case HOME:
      T.cursx = 0;
      break;

    case END:
      T.cursx = T.screencols - 1;
      break;

    case PG_U:
    case PG_D:
      {
        int cycles = T.screenrows;
        while (cycles--) {
          moveCursor(c == PG_U ? ARR_U : ARR_D);
      }
      break;
    }

    case  ARR_U: 
    case  ARR_D:
    case  ARR_R:
    case  ARR_L:
      moveCursor(c);
      break;
  }
}

/**
 * @brief Get the Cursor Pos object
 * @param rows 
 * @param cols 
 * @return int 
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
 * @brief Get the Window Size object
 * @param cols 
 * @return int return code
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
 * @brief Initialize the editor viewport
 */
void initEditor(void) {
  T.cursx = 0;
  T.cursy = 0;

  if (getWindowSize(&T.screenrows, &T.screencols) == -1) {
    panic("getWindowSize");
  } 
}
