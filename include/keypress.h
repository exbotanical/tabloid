#ifndef KEYPRESS_H
#define KEYPRESS_H

#define CTRL_KEY(k)                         ((k) & 0x1F)

// https://vt100.net/docs/vt100-ug/chapter3.html
#define ESCAPE_SEQ_CHAR                     '\x1b'
#define ESCAPE_SEQ                          "\x1b"
#define ESCAPE_SEQ_CLEAR_SCREEN             ESCAPE_SEQ "[2J"
#define ESCAPE_SEQ_CLEAR_SCROLLBUF          ESCAPE_SEQ "[3J"
#define ESCAPE_SEQ_CURSOR_POS               ESCAPE_SEQ "[H"
#define ESCAPE_SEQ_CURSOR_POS_FMT           ESCAPE_SEQ "[%d;%dH"

#define ESCAPE_SEQ_CURSOR_MAX_FWD           ESCAPE_SEQ "[999C"
#define ESCAPE_SEQ_CURSOR_MAX_DWN           ESCAPE_SEQ "[999B"
#define ESCAPE_SEQ_CURSOR_HIDE              ESCAPE_SEQ "[?25l"
#define ESCAPE_SEQ_CURSOR_SHOW              ESCAPE_SEQ "[?25h"
#define ESCAPE_SEQ_ERASE_LN_RIGHT_OF_CURSOR ESCAPE_SEQ "[K"

#define ESCAPE_SEQ_INVERT_COLOR             ESCAPE_SEQ "[7m"
#define ESCAPE_SEQ_NORM_COLOR               ESCAPE_SEQ "[m"

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

  PAGE_UP,
  PAGE_DOWN,
  HOME,
  END,

  DELETE
} Key;

void keypress_handle(void);

#endif /* KEYPRESS_H */
