#include <termios.h>
#include <time.h>

struct appendBuf {
  char *buf;
  int len;
};

static const char APPNAME[] = "cnano editor";

static const char APPVERSION[] = "0.0.1";

static const char ESCAPE = '\x1b';

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
  struct termios og_tty; /**< Pointer ref for storing original termios configurations */
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

extern struct ttyConfig T;
