#include "error.h"

#include "editor.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * @brief Exit the program with return code 1
 * 
 * @param s message
 */
void panic(const char *s) {
  // repos cursor on error so we don't inundate the user
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  perror(s);
  exit(1);
}
