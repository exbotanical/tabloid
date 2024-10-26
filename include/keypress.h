#ifndef KEYPRESS_H
#define KEYPRESS_H

#define CTRL_KEY(k)                      ((k) & 0x1F)

// https://vt100.net/docs/vt100-ug/chapter3.html
#define ESC_SEQ_CHAR                     '\x1b'
#define ESC_SEQ                          "\x1b"
#define ESC_SEQ_CLEAR_SCREEN             ESC_SEQ "[2J"
#define ESC_SEQ_CLEAR_SCROLLBUF          ESC_SEQ "[3J"
#define ESC_SEQ_CURSOR_POS               ESC_SEQ "[H"
#define ESC_SEQ_CURSOR_POS_FMT           ESC_SEQ "[%d;%dH"

#define ESC_SEQ_CURSOR_MAX_FWD           ESC_SEQ "[999C"
#define ESC_SEQ_CURSOR_MAX_DWN           ESC_SEQ "[999B"
#define ESC_SEQ_CURSOR_HIDE              ESC_SEQ "[?25l"
#define ESC_SEQ_CURSOR_SHOW              ESC_SEQ "[?25h"
#define ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR ESC_SEQ "[K"

#define ESC_SEQ_INVERT_COLOR             ESC_SEQ "[7m"
#define ESC_SEQ_NORM_COLOR               ESC_SEQ "[m"

#define ESC_SEQ_COLOR(num)               ESC_SEQ "[38;5;" #num "m"
#define ESC_SEQ_BG_COLOR(num)            ESC_SEQ "[48;5;" #num "m"

typedef enum {
  BACKSPACE  = 127,

  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,

  CTRL_ARROW_LEFT,
  CTRL_ARROW_RIGHT,
  CTRL_ARROW_UP,
  CTRL_ARROW_DOWN,

  SHIFT_ARROW_LEFT,
  SHIFT_ARROW_RIGHT,
  SHIFT_ARROW_UP,
  SHIFT_ARROW_DOWN,

  CTRL_SHIFT_ARROW_LEFT,
  CTRL_SHIFT_ARROW_RIGHT,
  CTRL_SHIFT_ARROW_UP,
  CTRL_SHIFT_ARROW_DOWN,

  PAGE_UP,
  PAGE_DOWN,
  HOME,
  END,

  ENTER,
  DELETE,

  CTRL_A,
  CTRL_E,
  CTRL_Q,
  CTRL_U,

  UNKNOWN
} Key;

void keypress_handle(void);

#endif /* KEYPRESS_H */
