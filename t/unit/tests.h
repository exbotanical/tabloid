#ifndef TESTS_H
#define TESTS_H

#include "editor.h"
#include "file.h"
#include "globals.h"
#include "libtap/libtap.h"
#include "test_utils.h"

void run_calc_tests(void);
void run_cursor_tests(void);
void run_piece_table_tests(void);
void run_line_buffer_tests(void);
void run_line_editor_tests(void);
void run_regression_tests(void);
void run_status_bar_tests(void);
void run_command_bar_tests(void);
void run_file_mgmt_tests(void);

#endif /* TESTS_H */
