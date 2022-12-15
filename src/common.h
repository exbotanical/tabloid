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

// Mandate the ctrl binding that exits the program by setting
// upper 3 bits to 0
#define CTRL_KEY(k) ((k)&0x1f)

#define NEQ_1(n) (n != 1)

static const char APP_NAME[] = "tabloid editor";

static const char APP_VERSION[] = "0.0.2";

static const char NULL_TERMINATOR = '\0';

static const char ESCAPE = '\x1b';

static const int CONFIRM_QUIT_X = 3;

/**
 * @brief Key mappings
 */
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

/**
 * @brief Syntax entities
 */
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
typedef struct row_t {
  /**
   * @brief Track a row's index
   */
  int idx;

  /**
   * @brief Track whether we're in a non-closed ml comment
   */
  int hl_open_comment;

  /**
   * @brief Store row size
   */
  int size;

  /**
   * @brief Store tab size
   */
  int rsize;

  /**
   * @brief Store row text
   */
  char* chars;

  /**
   * @brief Store tab contents
   */
  char* render;

  /**
   * @brief Store line syntax highlighting instructions.
   * Char array of int 0-255; each el corresponds to a char in `render`
   */
  unsigned char* highlight;
} Row;

typedef struct syntax_config_t {
  /**
   * @brief File type that will be displayed to the user in the status bar
   */
  char* f_type;

  /**
   * @brief An array of char arrays where ea str contains a pattern
   * to match a file name against
   */
  char** f_match;

  /**
   * @brief An array of char arrays where ea str contains a keyword
   * match; kw2 is terminated with a pipe char
   */
  char** keywords;

  /**
   * @brief Track how a single-line comment begins for the given file type
   */
  char* single_ln_comment_begin;

  /**
   * @brief Track how a multi-line comment begins for the given file type
   */
  char* multi_ln_comment_begin;

  /**
   * @brief Track how a multi-line comment ends for the given file type
   */
  char* multi_ln_comment_end;

  /**
   * @brief Bit field indicating whether to highlight numbers,
   * string for given file type
   */
  int flags;
} SyntaxConfig;

struct TtyConfig {
  /**
   * @brief Pointer ref for storing original termios configurations
   */
  struct termios og_tty;

  int screen_rows;

  int screencols;

  /**
   * @brief Cursor indices - chars on Cartesian plane
   */
  int curs_x, curs_y;

  /**
   * @brief Index of render buffer
   */
  int render_x;

  /**
   * @brief Row offset - tracks which row of the file the user is
   * scrolled to
   */
  int row_offset;

  /**
   * @brief Column offset - tracks horizontal cursor position
   */
  int col_offset;

  int num_rows;

  Row* row;

  /**
   * @brief The current filename, if extant
   */
  char* filename;

  char statusmsg[80];

  time_t statusmsg_time;

  /**
   * @brief Track file state
   */
  int dirty;

  SyntaxConfig* syntax;
};

extern struct TtyConfig T;

#endif
