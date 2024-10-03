#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>

#include "debug.h"
#include "editor.h"
#include "globals.h"
#include "keypress.h"

editor_t      editor;
file_handle_t logger;

int
main (int argc, char const *argv[]) {
  logger = (file_handle_t){
    .open  = logger_open,
    .close = logger_close,
    .read  = logger_read,
    .write = logger_write,
  };

  logger.open();
  atexit(logger.close);

  tty_enable_raw_mode();
  atexit(window_clear);
  atexit(tty_disable_raw_mode);

  editor_init();

  if (argc >= 2) {
    editor_open(argv[1]);
  }

  while (true) {
    window_refresh();
    keypress_handle();
  }

  return 0;
}
