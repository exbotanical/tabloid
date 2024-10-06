
#include "tests.h"

editor_t      editor;
file_handle_t logger;

int
main () {
  plan(156);

  run_cursor_tests();

  done_testing();
}
