#include "keypress.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "command_bar.h"
#include "cursor.h"
#include "exception.h"
#include "globals.h"
#include "line_editor.h"

typedef enum {
  KEYPRESS_SHIFT = 1,
  KEYPRESS_CTRL  = 2,
} keypress_flags_t;

static int CTRL_SHIFT_UP_MAP[] = {
  // 0 -> None
  ESC_SEQ_CHAR,
  // 1 -> Shift
  SHIFT_ARROW_UP,
  // 2 -> Ctrl
  CTRL_ARROW_UP,
  // 3 -> Ctrl+Shift
  CTRL_SHIFT_ARROW_UP,
};

static int CTRL_SHIFT_DOWN_MAP[] = {
  ESC_SEQ_CHAR,
  SHIFT_ARROW_DOWN,
  CTRL_ARROW_DOWN,
  CTRL_SHIFT_ARROW_DOWN,
};

static int CTRL_SHIFT_RIGHT_MAP[] = {
  ESC_SEQ_CHAR,
  SHIFT_ARROW_RIGHT,
  CTRL_ARROW_RIGHT,
  CTRL_SHIFT_ARROW_RIGHT,
};

static int CTRL_SHIFT_LEFT_MAP[] = {
  ESC_SEQ_CHAR,
  SHIFT_ARROW_LEFT,
  CTRL_ARROW_LEFT,
  CTRL_SHIFT_ARROW_LEFT,
};

static int
keypress_read (unsigned int* flags) {
  int  bytes_read;
  char c;

  while ((bytes_read = read(STDIN_FILENO, &c, 1)) != 1) {
    if (bytes_read == -1 && errno != EAGAIN) {
      panic("read failed and returned %d\n", bytes_read);
    }
  }

  // If the char is an escape sequence...
  if (c == ESC_SEQ_CHAR) {
    char seq[5];

    // Nothing else; just an esc seq literal
    if (read(STDIN_FILENO, &seq[0], 1) != 1) {
      return ESC_SEQ_CHAR;
    }
    if (read(STDIN_FILENO, &seq[1], 1) != 1) {
      return ESC_SEQ_CHAR;
    }

    // If the first byte is a [...
    if (seq[0] == '[') {
      // If the second byte is a digit...
      if (seq[1] >= '0' && seq[1] <= '9') {
        if (read(STDIN_FILENO, &seq[2], 1) != 1) {
          return ESC_SEQ_CHAR;
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
            return ESC_SEQ_CHAR;
          }

          switch (seq[3]) {
            case '2': *flags |= KEYPRESS_SHIFT; break;
            case '5': *flags |= KEYPRESS_CTRL; break;
            case '6':
              *flags |= KEYPRESS_SHIFT;
              *flags |= KEYPRESS_CTRL;
              break;
          }

          if (read(STDIN_FILENO, &seq[4], 1) != 1) {
            return ESC_SEQ_CHAR;
          }

          switch (seq[4]) {
            case 'A': {
              return CTRL_SHIFT_UP_MAP[*flags];
            }

            case 'B': {
              return CTRL_SHIFT_DOWN_MAP[*flags];
            }

            case 'C': {
              return CTRL_SHIFT_RIGHT_MAP[*flags];
            }

            case 'D': {
              return CTRL_SHIFT_LEFT_MAP[*flags];
            }
          }
        }
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

    return ESC_SEQ_CHAR;
  }

  switch (c) {
    case CTRL_KEY('a'): return CTRL_A;
    case CTRL_KEY('c'): return CTRL_C;
    case CTRL_KEY('e'): return CTRL_E;
    case CTRL_KEY('u'): return CTRL_U;
    case CTRL_KEY('q'): return CTRL_Q;
    case CTRL_KEY('z'): return CTRL_Z;
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

static void
keypress_handle_edit_mode_key (int c) {
  switch (c) {
    case ENTER: {
      line_editor_insert_newline(&editor.line_ed);
      break;
    }

    case CTRL_C: {
      mode_chmod(COMMAND_MODE);
      break;
    }

    case PAGE_UP: {
      cursor_move_visible_top(&editor.line_ed);
      break;
    }
    case PAGE_DOWN: {
      cursor_move_visible_bottom(&editor.line_ed);
      break;
    }

    case ARROW_UP: {
      cursor_move_up(&editor.line_ed);
      break;
    }
    case ARROW_DOWN: {
      cursor_move_down(&editor.line_ed);
      break;
    }

    case CTRL_ARROW_UP: {
      cursor_move_up(&editor.line_ed);
      break;
    }
    case CTRL_ARROW_DOWN: {
      cursor_move_down(&editor.line_ed);
      break;
    }

    case SHIFT_ARROW_LEFT: {
      cursor_select_left(&editor.line_ed);
      break;
    }
    case SHIFT_ARROW_RIGHT: {
      cursor_select_right(&editor.line_ed);
      break;
    }
    case SHIFT_ARROW_UP: {
      cursor_select_up(&editor.line_ed);
      break;
    }
    case SHIFT_ARROW_DOWN: {
      cursor_select_down(&editor.line_ed);
      break;
    }

    case CTRL_SHIFT_ARROW_LEFT: cursor_select_left_word(&editor.line_ed); break;
    case CTRL_SHIFT_ARROW_RIGHT: cursor_select_right_word(&editor.line_ed); break;
    case CTRL_SHIFT_ARROW_UP: break;
    case CTRL_SHIFT_ARROW_DOWN: break;

    default: {
      line_editor_insert_char(&editor.line_ed, c);
      break;
    }
  }

  cursor_snap_to_end(&editor.line_ed);
}

// TODO: Optimize
static void
keypress_handle_command_mode_key (int c) {
  switch (c) {
    case CTRL_C: {
      mode_chmod(EDIT_MODE);
      break;
    }

    case ENTER: {
      command_bar_process_command(&editor.c_bar);
      break;
    }

    default: {
      line_editor_insert_char(&editor.c_bar, c);
      break;
    }
  }
}

static void
keypress_handle_command_message_mode (int c) {
  switch (c) {
    case CTRL_C: {
      mode_chmod(EDIT_MODE);
      break;
    }
  }
}

void
keypress_handle (void) {
  unsigned int flags = 0;
  int          c     = keypress_read(&flags);

  if (editor.mode == COMMAND_MODE && editor.cmode == CB_MESSAGE) {
    keypress_handle_command_message_mode(c);
    return;
  }

  bool select_clear = (flags & KEYPRESS_SHIFT) != KEYPRESS_SHIFT;
  if (select_clear) {
    cursor_select_clear(&editor.line_ed);
  }

  line_editor_t* line_ed = editor.mode == EDIT_MODE ? &editor.line_ed : &editor.c_bar;

  switch (c) {
    case UNKNOWN: break;

    case BACKSPACE: line_editor_delete_char(line_ed); break;

    case CTRL_A: cursor_move_begin(line_ed); break;
    case CTRL_E: cursor_move_end(line_ed); break;
    case CTRL_U: line_editor_delete_line_before_x(line_ed); break;
    case CTRL_Z: line_editor_undo(line_ed); break;

    case DELETE: {
      cursor_move_right(line_ed);
      line_editor_delete_char(line_ed);
      break;
    }

    case HOME: {
      cursor_move_begin(line_ed);
      break;
    }
    case END: {
      cursor_move_end(line_ed);
      break;
    }

    case ARROW_LEFT: {
      cursor_move_left(line_ed);
      break;
    }
    case ARROW_RIGHT: {
      cursor_move_right(line_ed);
      break;
    }

    case CTRL_ARROW_LEFT: {
      cursor_move_left_word(line_ed);
      break;
    }
    case CTRL_ARROW_RIGHT: {
      cursor_move_right_word(line_ed);
      break;
    }

    default: {
      switch (editor.mode) {
        case EDIT_MODE: {
          keypress_handle_edit_mode_key(c);
          break;
        }

        case COMMAND_MODE: {
          keypress_handle_command_mode_key(c);
          break;
        }
      }
      break;
    }
  }
}
