/**
 * @file keypress.c
 * @author goldmund
 * @brief Handles keypress processing
 * @version 0.1
 * @date 2021-07-05
 *
 * @copyright Copyright (c) 2021 Matthew Zito (goldmund)
 *
 */

#include "keypress.h"

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "error.h"
#include "io.h"
#include "render.h"
#include "search.h"
#include "stream.h"
#include "viewport.h"

/**
 * @brief Wait for keypress from stdin and passthrough
 *
 * @return int returned, keypress enum or escape sequence
 */
int keypress_read(void) {
  int nread;
  char c;

  while (NEQ_1(nread = read(STDIN_FILENO, &c, 1))) {
    // ignore EAGAIN as cygwin returns -1 here, because...Windows
    if (nread == -1 && errno != EAGAIN) {
      panic("read");
    }
  }

  // read the given escape sequence
  if (c == ESCAPE) {
    char seq[3];

    if (NEQ_1(read(STDIN_FILENO, &seq[0], 1))) {
      return ESCAPE;
    }

    if (NEQ_1(read(STDIN_FILENO, &seq[1], 1))) {
      return ESCAPE;
    }

    if (seq[0] == '[') {
      if (seq[1] >= 0 && seq[1] <= '9') {
        if (NEQ_1(read(STDIN_FILENO, &seq[2], 1))) return ESCAPE;
        if (seq[2] == '~') {
          switch (seq[1]) {
            case '1':
              return HOME;
            case '3':
              return DEL;
            case '4':
              return END;
            case '5':
              return PG_U;
            case '6':
              return PG_D;
            case '7':
              return HOME;
            case '8':
              return END;
          }
        }
      } else {
        switch (seq[1]) {
          case 'A':
            return ARR_U;
          case 'B':
            return ARR_D;
          case 'C':
            return ARR_R;
          case 'D':
            return ARR_L;
          case 'H':
            return HOME;
          case 'F':
            return END;
        }
      }
    } else if (seq[0] == 'O') {
      switch (seq[1]) {
        case 'H':
          return HOME;
        case 'F':
          return END;
      }
    }

    return ESCAPE;
  } else {
    return c;
  }
}

/**
 * @brief Process keypresses by mapping various ctrl keys et al to tty
 * functionality
 */
void keypress_process(void) {
  int c = keypress_read();
  static int quit_x = CONFIRM_QUIT_X;

  switch (c) {
    case '\r':
      insert_nl();
      break;

    case CTRL_KEY('c'):
      if (T.dirty && quit_x > 0) {
        set_status_msg(
            "File has unsaved changes - press Ctrl-c %d more times to quit",
            quit_x);

        quit_x--;
        return;
      }
      // clean
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);

      exit(0);
      break;

    case CTRL_KEY('s'):
      f_write();
      break;

    case HOME:
      T.curs_x = 0;
      break;

    case END:
      if (T.curs_y < T.num_rows) {
        T.curs_x = T.row[T.curs_y].size;
      }
      break;

    // search
    case CTRL_KEY('f'):
      search();
      break;

    case BACKSPACE:
    case CTRL_KEY('h'):
    case DEL:
      // del char to right of cursor
      if (c == DEL) {
        cursor_mv(ARR_R);
      }
      rm_char();
      break;

    // pos cursor at top or bottom of viewport
    case PG_U:
    case PG_D: {
      if (c == PG_U) {
        T.curs_y = T.row_offset;
      } else if (c == PG_D) {
        T.curs_y = T.row_offset + T.screen_rows - 1;

        if (T.curs_y > T.num_rows) T.curs_y = T.num_rows;
      }

      int cycles = T.screen_rows;
      while (cycles--) {
        cursor_mv(c == PG_U ? ARR_U : ARR_D);
      }
      break;
    }

    case ARR_U:
    case ARR_D:
    case ARR_R:
    case ARR_L:
      cursor_mv(c);
      break;

    // ignore unimplemented escape sequences
    // ignore C-l, as this editor refreshes after ea keypress
    case CTRL_KEY('l'):
    case '\x1b':
      break;

    default:
      insert_char(c);
      break;
  }

  quit_x = CONFIRM_QUIT_X;
}
