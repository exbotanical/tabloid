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
  ok(rs->num_lines == 4, "num_lines field is correct");

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
test_line_buffer_newline (void) {
  char*           raw = "hello\nworld\nthis\nis a line";
  render_state_t* rs  = render_state_init(raw);
  render_state_refresh(rs);

  char nl[1];
  nl[0] = '\n';
  nl[1] = '\0';
  // TODO: we gotta make sure if the user types a literal \0, we don't accidentally newline on 'em
  render_state_insert(rs, 9, 3, nl);

  ok(array_size(rs->line_info) == 5, "has 5 lines");
  ok(rs->num_lines == 5, "num_lines field is correct");

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
test_line_buffer_newlines_only (void) {
  render_state_t* rs = render_state_init(NULL);
  render_state_refresh(rs);

  char nl[1];
  nl[0] = '\n';
  nl[1] = '\0';
  // TODO: we gotta make sure if the user types a literal \0, we don't accidentally newline on 'em
  render_state_insert(rs, 0, 0, nl);
  render_state_insert(rs, 0, 1, nl);

  ok(array_size(rs->line_info) == 3, "has 3 lines");
  ok(rs->num_lines == 3, "num_lines field is correct");

  line_info_t* li = array_get(rs->line_info, 0);
  ok(li->line_start == 0, "correct line start 1");
  ok(li->line_length == 0, "correct line length 1");

  li = array_get(rs->line_info, 1);
  ok(li->line_start == 1, "correct line start 2");
  ok(li->line_length == 0, "correct line length 2");

  li = array_get(rs->line_info, 2);
  ok(li->line_start == 2, "correct line start 3");
  ok(li->line_length == 0, "correct line length 3");

  render_state_free(rs);
}

void
test_line_buffer_get_line (void) {
  char*           raw = "hello\nworld\nthis\nis a line";
  render_state_t* rs  = render_state_init(raw);
  render_state_refresh(rs);

  ok(array_size(rs->line_info) == 4, "has 4 lines");
  ok(rs->num_lines == 4, "num_lines field is correct");

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
  ok(rs->num_lines == 4, "num_lines field is correct");

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
  ok(rs->num_lines == 4, "num_lines field is correct");

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
  ok(rs->num_lines == 4, "num_lines field is correct");

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
  ok(rs->num_lines == 3, "num_lines field is correct");

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
test_line_buffer_type_then_delete (void) {
  render_state_t* rs = render_state_init(NULL);
  render_state_refresh(rs);
  render_state_insert(rs, 0, 0, "d");
  render_state_insert(rs, 1, 0, "d");
  render_state_insert(rs, 2, 0, "d");
  render_state_delete(rs, 2, 0);

  ok(array_size(rs->line_info) == 1, "has 1 line");
  ok(rs->num_lines == 1, "num_lines field is correct");

  char buffer[100];
  render_state_get_line(rs, 0, buffer);
  is(buffer, "dd", "correct end state");
}

void
test_line_buffer_type_then_delete_earlier_pos (void) {
  // In this scenario, the user types a few characters, then moves the cursor back 1 space and deletes.
  render_state_t* rs = render_state_init(NULL);
  render_state_refresh(rs);
  render_state_insert(rs, 0, 0, "d");
  render_state_insert(rs, 1, 0, "d");
  render_state_insert(rs, 2, 0, "d");
  render_state_delete(rs, 1, 0);
  render_state_delete(rs, 0, 0);

  ok(array_size(rs->line_info) == 1, "has 1 line");
  ok(rs->num_lines == 1, "num_lines field is correct");

  char buffer[100];
  render_state_get_line(rs, 0, buffer);
  is(buffer, "d", "correct end state");
}

void
test_line_buffer_insert_line_on_first (void) {
  render_state_t* rs = render_state_init(NULL);
  render_state_refresh(rs);
  render_state_insert(rs, 0, 0, "hello\n");
  render_state_insert(rs, 0, 0, "hello\n");

  ok(array_size(rs->line_info) == 3, "has 2 lines");
  ok(rs->num_lines == 3, "num_lines field is correct");

  char buffer[100];

  render_state_get_line(rs, 0, buffer);
  is(buffer, "hello", "correct end state");
  memset(buffer, 0, 100);

  render_state_get_line(rs, 1, buffer);
  is(buffer, "hello", "correct end state");
  memset(buffer, 0, 100);

  render_state_get_line(rs, 2, buffer);
  is(buffer, "", "correct end state");
}

void
run_line_buffer_tests (void) {
  test_line_buffer();
  test_line_buffer_newline();
  test_line_buffer_newlines_only();
  test_line_buffer_get_line();

  // The following tests are real scenarios translated to unit tests
  test_line_buffer_type_then_delete();
  test_line_buffer_type_then_delete_earlier_pos();
  test_line_buffer_insert_line_on_first();
}
