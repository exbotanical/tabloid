
#include "tests.h"

editor_t      editor;
file_handle_t logger;

int
main () {
  plan(782);

  run_calc_tests();
  run_cursor_tests();
  run_piece_table_tests();
  run_line_buffer_tests();
  run_editor_tests();
  run_regression_tests();
  run_status_bar_tests();
  run_file_mgmt_tests();

  done_testing();
}
