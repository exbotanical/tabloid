#include "keypress.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cursor.h"
#include "editor.h"
#include "exception.h"
#include "globals.h"
#include "window.h"

static int
keypress_read (void) {
  int  bytes_read;
  char c;

  while ((bytes_read = read(STDIN_FILENO, &c, 1)) != 1) {
    if (bytes_read == -1 && errno != EAGAIN) {
      panic("read failed and returned %d\n", bytes_read);
    }
  }

  // If the char is an escape sequence...
  if (c == ESCAPE_SEQ_CHAR) {
    char seq[5];

    // Nothing else; just an esc seq literal
    if (read(STDIN_FILENO, &seq[0], 1) != 1) {
      return ESCAPE_SEQ_CHAR;
    }
    if (read(STDIN_FILENO, &seq[1], 1) != 1) {
      return ESCAPE_SEQ_CHAR;
    }

    // If the first byte is a [...
    if (seq[0] == '[') {
      // If the second byte is a digit...
      if (seq[1] >= '0' && seq[1] <= '9') {
        if (read(STDIN_FILENO, &seq[2], 1) != 1) {
          return ESCAPE_SEQ_CHAR;
        }

        if (seq[2] == '~') {
          switch (seq[1]) {
            case '3': return DELETE;
            case '1':
            case '7': return HOME;
            case '4':
            case '8': return END;
            case '5': return PAGE_UP;
            case '6': return PAGE_DOWN;
          }
        }

        if (seq[2] == ';') {
          if (read(STDIN_FILENO, &seq[3], 1) != 1) {
            return ESCAPE_SEQ_CHAR;
          }

          // ESC-[1;5D
          if (seq[3] == '5') {
            if (read(STDIN_FILENO, &seq[4], 1) != 1) {
              return ESCAPE_SEQ_CHAR;
            }

            switch (seq[4]) {
              case 'A': return CTRL_ARROW_UP;
              case 'B': return CTRL_ARROW_DOWN;
              case 'C': return CTRL_ARROW_RIGHT;
              case 'D': return CTRL_ARROW_LEFT;
            }
          }
        }

        // HERE!
      } else {
        switch (seq[1]) {
          case 'A': return ARROW_UP;
          case 'B': return ARROW_DOWN;
          case 'C': return ARROW_RIGHT;
          case 'D': return ARROW_LEFT;
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

    return ESCAPE_SEQ_CHAR;
  }

  switch (c) {
    case CTRL_KEY('a'): return CTRL_A;
    case CTRL_KEY('e'): return CTRL_E;
    case CTRL_KEY('q'): return CTRL_Q;
    // Ctrl+h aka ctrl code 8 aka backspace
    case CTRL_KEY('h'): return BACKSPACE;
  }

  if (c == '\r') {
    return ENTER;
  }

  // Ignore unknown ctrl sequences
  if (CTRL_KEY(c) == c) {
    return UNKNOWN;
  }

  return c;
}

void
keypress_handle (void) {
  int c = keypress_read();

  switch (c) {
    case UNKNOWN: break;
    case CTRL_Q: exit(0);

    case ENTER: editor_insert_newline(); break;
    case BACKSPACE: editor_del_char(); break;

    case CTRL_A: cursor_move_begin(); break;
    case CTRL_E: cursor_move_end(); break;

    case DELETE:
      cursor_move_right();
      editor_del_char();
      break;

    case PAGE_UP: {
      cursor_move_visible_top();
      break;
    }
    case PAGE_DOWN: {
      cursor_move_visible_bottom();
      break;
    }
    case HOME: {
      cursor_move_begin();
      break;
    }
    case END: {
      cursor_move_end();
      break;
    }

    case ARROW_LEFT: {
      cursor_move_left();
      break;
    }
    case ARROW_RIGHT: {
      cursor_move_right();
      break;
    }
    case ARROW_UP: {
      cursor_move_up();
      break;
    }
    case ARROW_DOWN: {
      cursor_move_down();
      break;
    }

    case CTRL_ARROW_LEFT: {
      cursor_move_left_word();
      break;
    }
    case CTRL_ARROW_RIGHT: {
      cursor_move_right_word();
      break;
    }
    case CTRL_ARROW_UP: {
      cursor_move_up();
      break;
    }
    case CTRL_ARROW_DOWN: {
      cursor_move_down();
      break;
    }

    default: {
      editor_insert_char(c);
      break;
    }
  }

  cursor_snap_to_end();
}
