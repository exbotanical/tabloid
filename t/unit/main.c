
#include "tests.h"

editor_t      editor;
file_handle_t logger;

int
main () {
  plan(314);

  run_calc_tests();
  run_cursor_tests();
  run_piece_table_tests();
  run_line_buffer_tests();
  run_editor_tests();

  done_testing();
}
