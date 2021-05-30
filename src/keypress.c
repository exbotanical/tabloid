#include "keypress.h"

#include "common.h"
#include "error.h"
#include "io.h"
#include "render.h"
#include "stream.h"
#include "viewport.h"

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

/* Local Macros */
#define CTRL_KEY(k) ((k) & 0x1f) /**< Mandate the ctrl binding that exits the program by setting upper 3 bits to 0 */

/**
 * @brief Wait for keypress from stdin and passthrough
 *
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
	static int quit_x = CONFIRM_QUIT_X;

  switch (c) {
		case '\r':
			break;

    case CTRL_KEY('c'):
			if (T.dirty && quit_x > 0) {
				setStatusMessage(
					"File has unsaved changes - press Ctrl-c %d more times to quit",
					quit_x
				);

				quit_x--;
				return;
			}
      // clean
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);

      exit(0);
      break;

		case CTRL_KEY('s'):
			saveToFile();
			break;

    case HOME:
      T.cursx = 0;
      break;

    case END:
      if (T.cursy < T.numrows) {
        T.cursx = T.row[T.cursy].size;
      }
      break;

		case BACKSPACE:
		case CTRL_KEY('h'):
		case DEL:
			// del char to right of cursor
			if (c == DEL) moveCursor(ARR_R);
			delChar();
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

    case ARR_U:
    case ARR_D:
    case ARR_R:
    case ARR_L:
      moveCursor(c);
      break;

		// ignore unimplemented escape sequences
		// ignore C-l, as this editor refreshes after ea keypress
		case CTRL_KEY('l'):
		case '\x1b':
			break;

		default:
			insertChar(c);
			break;
  }

	quit_x = CONFIRM_QUIT_X;
}
