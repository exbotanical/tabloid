#include "line_buffer.h"

#include <stdlib.h>

#include "piece_table.h"
#include "tests.h"

void
test_line_buffer (void) {
  char*           raw = "hello\nworld\nthis\nis a line";
  render_state_t* rs  = render_state_init(raw);
  render_state_refresh(rs);

  ok(array_size(rs->line_info) == 4, "has 4 lines");
  line_info_t* li = array_get(rs->line_info, 0);
  ok(li->line_start == 0, "correct line start 1");
  ok(li->line_length == 5, "correct line length 1");

  li = array_get(rs->line_info, 1);
  ok(li->line_start == 6, "correct line start 2");
  ok(li->line_length == 5, "correct line length 2");

  li = array_get(rs->line_info, 2);
  ok(li->line_start == 12, "correct line start 3");
  ok(li->line_length == 4, "correct line length 3");

  li = array_get(rs->line_info, 3);
  ok(li->line_start == 17, "correct line start 4");
  ok(li->line_length == 9, "correct line length 4");

  render_state_free(rs);
}

void
test_line_buffer_get_line (void) {
  char*           raw = "hello\nworld\nthis\nis a line";
  render_state_t* rs  = render_state_init(raw);
  render_state_refresh(rs);

  ok(array_size(rs->line_info) == 4, "has 4 lines");

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

  render_state_insert(rs, 5, 0, "x");

  ok(array_size(rs->line_info) == 4, "has 4 lines");

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

  render_state_insert(rs, 1, 1, "x");

  render_state_insert(rs, 3, 1, "x");

  render_state_insert(rs, 3, 3, "x");

  render_state_insert(rs, 5, 3, "x");

  ok(array_size(rs->line_info) == 4, "has 4 lines");

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

  render_state_delete(rs, 5, 0);

  render_state_delete(rs, 2, 1);

  render_state_delete(rs, 4, 3);

  render_state_insert(rs, 3, 3, "x");

  ok(array_size(rs->line_info) == 4, "has 4 lines");

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

  render_state_delete(rs, -1, 1);

  ok(array_size(rs->line_info) == 3, "has 3 lines");

  render_state_get_line(rs, 0, buffer);
  is(buffer, "hellowxxrld", "returns the expected line");
  memset(buffer, 0, 100);

  render_state_get_line(rs, 1, buffer);
  is(buffer, "this", "returns the expected line");
  memset(buffer, 0, 100);

  render_state_get_line(rs, 2, buffer);
  is(buffer, "is xxx line", "returns the expected line");
  memset(buffer, 0, 100);

  render_state_free(rs);
}

void
run_line_buffer_tests (void) {
  test_line_buffer();
  test_line_buffer_get_line();
}
