#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#define DEFAULT_CURSOR_STATE                                                                                      \
  (cursor_t) {                                                                                                    \
    .col_off = 0, .row_off = 0, .x = 0, .y = 0, .select_active = false, .select_anchor = -1, .select_offset = -1, \
  }

#define SET_CURSOR(_x, _y) \
  editor.curs.x = _x;      \
  editor.curs.y = _y

#define PRINT_CURSOR()                                   \
  printf("(x=%d,y=%d)\n", editor.curs.x, editor.curs.y); \
  fflush(stdout)

#define CALL_N_TIMES(n, call) \
  for (unsigned int i = 0; i < n; i++) call

#endif /* TEST_UTILS_H */
