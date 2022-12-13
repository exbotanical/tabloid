#include <stdio.h>
#include <stdlib.h>

#include "editor.h"
#include "error.h"
#include "io.h"
#include "keypress.h"
#include "render.h"
#include "viewport.h"

int main(int argc, char* argv[]) {
  enable_raw_mode();
  editor_init();

  if (argc >= 2) {
    f_open(argv[1]);
  }

  set_status_msg("C-s to save | C-c to quit | C-f to search");

  while (1) {
    clear_screen();
    keypress_process();
  }

  return EXIT_SUCCESS;
}
