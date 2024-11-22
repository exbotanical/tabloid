#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#define DEFAULT_CURSOR_STATE                                                                                      \
  (cursor_t) {                                                                                                    \
    .col_off = 0, .row_off = 0, .x = 0, .y = 0, .select_active = false, .select_anchor = -1, .select_offset = -1, \
  }

#define SET_CURSOR(_x, _y)    \
  editor.line_ed.curs.x = _x; \
  editor.line_ed.curs.y = _y

#define PRINT_CURSOR()                                                   \
  printf("(x=%d,y=%d)\n", editor.line_ed.curs.x, editor.line_ed.curs.y); \
  fflush(stdout)

#define CALL_N_TIMES(n, call) \
  for (unsigned int i = 0; i < n; i++) call

// FILE *fp = fopen("./t/fixtures/tmp.txt", "rw+");
// write(fp->_fileno, buffer_state(buf), buffer_size(buf));

#endif /* TEST_UTILS_H */
