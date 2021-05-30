#include "editor.h"
#include "error.h"
#include "io.h"
#include "keypress.h"
#include "render.h"
#include "viewport.h"

#include <stdio.h>

int main(int argc, char *argv[]) {
  enableRawMode();
  initEditor();

  if (argc >= 2) {
    openFile(argv[1]);
  }

  setStatusMessage("HELP: Ctrl-s -> save | Ctrl-c -> quit");

  while (1) {
    clearScreen();
    procKeypress();
  }

  return 0;
}
