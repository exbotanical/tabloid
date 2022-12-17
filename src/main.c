#include <stdio.h>
#include <stdlib.h>

#include "constants.h"
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

  set_status_msg(INITIAL_VISUAL_MODE_PROMPT);

  while (1) {
    clear_screen();
    keypress_process();
  }

  return EXIT_SUCCESS;
}
