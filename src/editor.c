#include "editor.h"

#include "error.h"

#include <unistd.h>
#include <errno.h>

#define EDITOR_PREFIX "~" 
#define EDITOR_ROWS 24

/**
 * @brief Wait for keypress from stdin and passthrough
 * 
 * @return char returned, valid keypress
 */
char readKey(void) {
  int nread;
  char c;

  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    // ignore EAGAIN as cygwin returns -1 here, because...windows
    if (nread == -1 && errno != EAGAIN) {
      panic("read");
    }
  }
  
  return c;
}

/**
 * @brief Clear the user's screen and set cursor position
 * 
 * Performs cleanup by clearing the user's screen, setting the cursor position.
 * Utilized in lieu of `atexit` given this would also clear error messages produced by `panic` 
 * 
 * @see https://vt100.net/docs/vt100-ug/chapter3.html#ED
 * @see https://vt100.net/docs/vt100-ug/chapter3.html#CUP
 * 
 * @todo use ncurses for better term support
 */
void clearScreen(void) {
  // TODO use terminfo db
  write(STDOUT_FILENO, "\x1b[2J", 4);
  
  reposCursor();

  drawRows();
  
  reposCursor();
}

/**
 * @brief Set the cursor position at line 1, col 1
 * 
 */
void reposCursor(void) {
  write(STDOUT_FILENO, "\x1b[H", 3);
}

/**
 * @brief Draws vim-like tilde-prepended rows on the screen
 * 
 * @todo set to customizable with lineno
 * @todo util `EDITOR_PREFIX`
 * 
 */
void drawRows(void) {
  int line;

  for (line = 0; line < EDITOR_ROWS; line++) {
    write(STDOUT_FILENO, "~\r\n", 3);
  }
}
