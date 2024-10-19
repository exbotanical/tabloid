#include "line_buffer.h"

#include <stdlib.h>

#include "piece_table.h"
#include "tests.h"

void
test_line_buffer (void) {
  char*          raw = "hello\nworld\nthis\nis a line";
  piece_table_t* pt  = piece_table_init();
  piece_table_setup(pt, raw);

  render_state_t* rs = render_state_init();
  render_state_refresh(rs, pt);

  ok(array_size(rs->line_starts) == 5, "has 5 line starts");
  ok((int)array_get(rs->line_starts, 0) == 0, "correct line start");
  ok((int)array_get(rs->line_starts, 1) == 6, "correct line start");
  ok((int)array_get(rs->line_starts, 2) == 12, "correct line start");
  ok((int)array_get(rs->line_starts, 3) == 17, "correct line start");
  ok((int)array_get(rs->line_starts, 4) == 26, "correct line start");

  piece_table_free(pt);
  render_state_free(rs);
}

void
test_line_buffer_get_line (void) {
  char*          raw = "hello\nworld\nthis\nis a line";
  piece_table_t* pt  = piece_table_init();
  piece_table_setup(pt, raw);

  render_state_t* rs = render_state_init();
  render_state_refresh(rs, pt);

  ok(array_size(rs->line_starts) == 5, "has 4 lines (plus one to terminate last line)");

  char buffer[100];

  render_state_get_line(rs, 0, buffer);
  is(buffer, "hello", "returns the expected line");
  memset(buffer, 0, 100);

  render_state_get_line(rs, 1, buffer);
  is(buffer, "world", "returns the expected line");
  memset(buffer, 0, 100);

  render_state_get_line(rs, 2, buffer);
  is(buffer, "this", "returns the expected line");
  memset(buffer, 0, 100);

  render_state_get_line(rs, 3, buffer);
  is(buffer, "is a line", "returns the expected line");
  memset(buffer, 0, 100);

  // Now we test that it works after modifying the buffer.
  // We'll also simulate the cursor edits

  render_state_insert(rs, pt, 5, 0, "x");

  ok(array_size(rs->line_starts) == 5, "has 4 lines (plus one to terminate last line)");

  render_state_get_line(rs, 0, buffer);
  is(buffer, "hellox", "returns the expected line");
  memset(buffer, 0, 100);

  render_state_get_line(rs, 1, buffer);
  is(buffer, "world", "returns the expected line");
  memset(buffer, 0, 100);

  render_state_get_line(rs, 2, buffer);
  is(buffer, "this", "returns the expected line");
  memset(buffer, 0, 100);

  render_state_get_line(rs, 3, buffer);
  is(buffer, "is a line", "returns the expected line");
  memset(buffer, 0, 100);

  render_state_insert(rs, pt, 1, 1, "x");

  render_state_insert(rs, pt, 3, 1, "x");

  render_state_insert(rs, pt, 3, 3, "x");

  render_state_insert(rs, pt, 5, 3, "x");

  ok(array_size(rs->line_starts) == 5, "has 4 lines (plus one to terminate last line)");

  render_state_get_line(rs, 0, buffer);
  is(buffer, "hellox", "returns the expected line");
  memset(buffer, 0, 100);

  render_state_get_line(rs, 1, buffer);
  is(buffer, "wxoxrld", "returns the expected line");
  memset(buffer, 0, 100);

  render_state_get_line(rs, 2, buffer);
  is(buffer, "this", "returns the expected line");
  memset(buffer, 0, 100);

  render_state_get_line(rs, 3, buffer);
  is(buffer, "is xax line", "returns the expected line");
  memset(buffer, 0, 100);

  render_state_delete(rs, pt, 5, 0);

  render_state_delete(rs, pt, 2, 1);

  render_state_delete(rs, pt, 4, 3);

  render_state_insert(rs, pt, 3, 3, "x");

  ok(array_size(rs->line_starts) == 5, "has 4 lines (plus one to terminate last line)");

  render_state_get_line(rs, 0, buffer);
  is(buffer, "hello", "returns the expected line");
  memset(buffer, 0, 100);

  render_state_get_line(rs, 1, buffer);
  is(buffer, "wxxrld", "returns the expected line");
  memset(buffer, 0, 100);

  render_state_get_line(rs, 2, buffer);
  is(buffer, "this", "returns the expected line");
  memset(buffer, 0, 100);

  render_state_get_line(rs, 3, buffer);
  is(buffer, "is xxx line", "returns the expected line");
  memset(buffer, 0, 100);

  render_state_delete(rs, pt, -1, 1);

  ok(array_size(rs->line_starts) == 4, "has 3 lines (plus one to terminate last line)");

  render_state_get_line(rs, 0, buffer);
  is(buffer, "hellowxxrld", "returns the expected line");
  memset(buffer, 0, 100);

  render_state_get_line(rs, 1, buffer);
  is(buffer, "this", "returns the expected line");
  memset(buffer, 0, 100);

  render_state_get_line(rs, 2, buffer);
  is(buffer, "is xxx line", "returns the expected line");
  memset(buffer, 0, 100);
}

void
run_line_buffer_tests (void) {
  test_line_buffer();
  test_line_buffer_get_line();
}
