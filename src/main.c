#include "editor.h"
#include "error.h"
#include "keypress.h"
#include "render.h"
#include "viewport.h"

#include <stdio.h>

int main(int argc, char *argv[]) {
  enableRawMode();
  initEditor();

  if (argc >= 2) {
    editorOpen(argv[1]);
  }

  setStatusMessage("HELP: C-q -> quit");

  while (1) {
    clearScreen();
    procKeypress();
  }

  return 0;
}
