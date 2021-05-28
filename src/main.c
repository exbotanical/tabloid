#include "error.h"
#include "editor.h"

#include <stdio.h>

int main(void) {
  enableRawMode();
  initEditor();

  while (1) {
    clearScreen();
    procKeypress();
  }

  return 0;
}
