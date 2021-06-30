#include "editor.h"
#include "error.h"
#include "io.h"
#include "keypress.h"
#include "render.h"
#include "viewport.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  enable_rawmode();
  init_ed();

  if (argc >= 2) {
    f_open(argv[1]);
  }

  set_stats_msg("C-s to save | C-c to quit | C-f to search");

  while (1) {
    clear_screen();
    proc_keypress();
  }

  return EXIT_SUCCESS;
}
