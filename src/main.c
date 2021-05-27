#include "error.h"
#include "term.h"
#include "editor.h"

#include <stdio.h>

int main(void) {
  enableRawMode();

  while (1) {
    clearScreen();
    procKeyPress();
  }

  return 0;
}
