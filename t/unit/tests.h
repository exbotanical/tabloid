#ifndef TESTS_H
#define TESTS_H

#include "editor.h"
#include "file.h"
#include "globals.h"
#include "libtap/libtap.h"

#define DEFAULT_CURSOR_STATE                                                  \
  (cursor_t) {                                                                \
    .col_off = 0, .row_off = 0, .x = 0, .y = 0, .render_x = DEFAULT_LNPAD + 1 \
  }

#define SET_CURSOR(_x, _y) \
  editor.curs.x = _x;      \
  editor.curs.y = _y

void run_calc_tests(void);
void run_cursor_tests(void);
void run_piece_table_tests(void);
void run_line_buffer_tests(void);
void run_editor_tests(void);

#endif /* TESTS_H */
