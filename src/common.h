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

/* syntax classes */
enum highlights {
	HL_DEFAULT = 0,
	HL_COMMENT,
	HL_ML_COMMENT,
	HL_STRING,
	HL_NUMBER,
	HL_SEARCH_MATCH,
	HL_KEYWORD1,
	HL_KEYWORD2,
};

/**
 * @brief Stateful representation of a row buffer
 *
 * @todo Allow custom tab-size
 */
typedef struct t_row {
	int idx; /**< track a row's index */
	int hl_open_comment; /**< track whether we're in a non-closed ml comment */
  int size; /**< store row size */
  int rsize; /**< store tab size */
  char* chars; /**< store row text */
  char* render; /**< store tab contents */

	/* Store line syntax highlighting instructions */
	// char array of int 0-255; ea array item corresponds to a char in `render`
	unsigned char* highlight;
} t_row;

typedef struct syntax_config {
	char* f_type; /**< file type that will be displayed to the user in the status bar */
	char** f_match; /**< an array of char arrays where ea str contains a pattern to match a file name against */
	char** keywords; /**< an array of char arrays where ea str contains a keyword match; kw2 is terminated with a pipe char */
	char* single_ln_comment_begin; /**< track how a single-line comment begins for the given file type*/
	char *multi_ln_comment_begin;	/**< track how a multi-line comment begins for the given file type*/
	char *multi_ln_comment_end; /**< track how a multi-line comment ends for the given file type*/
	int flags; /**< bit field indicating whether to hl numbers, string for given file type */
} syntax_config;

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
	syntax_config *syntax;
};

extern struct tty_conf T;

#endif
