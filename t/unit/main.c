
#include "tests.h"

editor_t      editor;
file_handle_t logger;

int
main () {
  plan(1937);

  run_calc_tests();
  run_cursor_tests();
  run_piece_table_tests();
  run_line_buffer_tests();
  run_line_editor_tests();
  run_regression_tests();
  run_status_bar_tests();
  run_command_bar_tests();
  run_file_mgmt_tests();
  run_scanner_tests();
  run_lexer_tests();
  run_parser_tests();
  run_str_search_tests();

  done_testing();
}
