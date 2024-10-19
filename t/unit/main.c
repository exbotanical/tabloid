
#include "tests.h"

editor_t      editor;
file_handle_t logger;

int
main () {
  plan(203);

  run_cursor_tests();
  run_piece_table_tests();
  run_line_buffer_tests();
  run_calc_tests();

  done_testing();
}
