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
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

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

/* Feature Test Macros */
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

/* Local Macros */
#define CTRL_KEY(k) ((k) & 0x1f) /**< Mandate the ctrl binding that exits the program by setting upper 3 bits to 0 */
#define ABUF_INIT {NULL, 0} /**< Initialize an `appendbuf` */

/* Local Constants */
#define TAB_SIZE 8

/***********
 * State
 ***********/

/* Tty State */

/**
 * @brief Stateful representation of a row buffer
 * 
 * @todo Allow custom tab-size
 */
typedef struct trow {
  int size; /**< Store row size */
  int rsize; /**< Store tab size */
  char *chars; /**< Store row text */
  char *render; /**< Store tab contents */
} trow;

struct ttyConfig {
  struct termios og_tty; /**< Pointer ref for storing original, original termios configurations */
  int screenrows;
  int screencols;
  int cursx, cursy; /**< Cursor indices - chars on Cartesian plane */
  int renderx; /**< Index of render buffer */
  int rowoff; /**< Row offset - tracks which row of the file the user is scrolled to */
  int coloff; /**< Column offset - tracks horizontal cursor position */
  int numrows;
  trow *row;
  char *filename; /**< The current filename, if extant */
  char statusmsg[80];
  time_t statusmsg_time;
};

/* Mappings */
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

struct ttyConfig T;

/***********
 * Cursor
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

/***********
 * Viewport
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
 * @brief Draws vim-like tilde-prepended rows on the screen
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
    "%d%d",
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

/***************
 * ♎︎♎︎♎︎ Input ♎︎♎︎♎︎
 ***************/

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
      if (T.cursy < T.numrows) {
        T.cursx = T.row[T.cursy].size;
      }
      break;

    // pos cursor at top or bottom of viewport
    case PG_U:
    case PG_D:
      { 
        if (c == PG_U) {
          T.cursy = T.rowoff;
        } else if (c == PG_D) {
          T.cursy = T.rowoff + T.screenrows - 1;

          if (T.cursy > T.numrows) T.cursy = T.numrows;
        }
        
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
