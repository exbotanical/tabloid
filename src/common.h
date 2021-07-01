#ifndef COMMON_H
#define COMMON_H

#include <termios.h>
#include <time.h>

#ifdef DEBUG

#ifndef fprintf
#include <stdio.h>
#endif

#define LOG(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)

#else

#define LOG(fmt, ...)

#endif

#define TAB_SIZE 8

#define CTRL_KEY(k) ((k) & 0x1f) /**< Mandate the ctrl binding that exits the program by setting upper 3 bits to 0 */
#define NEQ_1(n) (n != 1) /**< Comparison helper */

struct extensible_buffer {
  char* buf;
  int len;
};

static const char APPNAME[] = "tabloid editor";

static const char APPVERSION[] = "0.0.2";

static const char NULL_TERM = '\0';

static const char ESCAPE = '\x1b';

static const int CONFIRM_QUIT_X = 3;

/* Mappings */
enum key_bindings {
	BACKSPACE = 127,
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

enum highlights {
	HL_DEFAULT = 0,
	HL_NUMBER,
	HL_SEARCH_MATCH,
};

/**
 * @brief Stateful representation of a row buffer
 *
 * @todo Allow custom tab-size
 */
typedef struct t_row {
  int size; /**< Store row size */
  int rsize; /**< Store tab size */
  char* chars; /**< Store row text */
  char* render; /**< Store tab contents */

	/* Store line syntax highlighting instructions */
	// char array of int 0-255; ea array item corresponds to a char in `render`
	unsigned char* highlight;
} t_row;

struct tty_conf {
  struct termios og_tty; /**< Pointer ref for storing original termios configurations */
  int screenrows;
  int screencols;
  int curs_x, curs_y; /**< Cursor indices - chars on Cartesian plane */
  int render_x; /**< Index of render buffer */
  int rowoff; /**< Row offset - tracks which row of the file the user is scrolled to */
  int coloff; /**< Column offset - tracks horizontal cursor position */
  int numrows;
  t_row* row;
  char* filename; /**< The current filename, if extant */
  char statusmsg[80];
  time_t statusmsg_time;
	int dirty; /**< Track file state */
};

extern struct tty_conf T;

#endif
