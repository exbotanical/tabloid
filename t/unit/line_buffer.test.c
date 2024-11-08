#include "line_buffer.h"

#include <stdlib.h>

#include "piece_table.h"
#include "tests.h"

static char buf[128];

static char*
get_line (line_buffer_t* rs, int lineno) {
  memset(buf, 0, 128);
  line_buffer_get_line(rs, lineno, buf);
  return buf;
}

static cursor_t*
create_test_cursor (int x, int y) {
  cursor_t* curs = malloc(sizeof(cursor_t));
  curs->x        = x;
  curs->y        = y;
  curs->col_off  = x;
  curs->row_off  = x;
  return curs;
}

static void
test_line_buffer (void) {
  char*          raw = "hello\nworld\nthis\nis a line";
  line_buffer_t* rs  = line_buffer_init(raw);
  line_buffer_refresh(rs);

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

  line_buffer_free(rs);
}

static void
test_line_buffer_newline (void) {
  char*          raw = "hello\nworld\nthis\nis a line";
  line_buffer_t* rs  = line_buffer_init(raw);
  line_buffer_refresh(rs);

  char nl[1];
  nl[0] = '\n';
  nl[1] = '\0';
  // TODO: we gotta make sure if the user types a literal \0, we don't
  // accidentally newline on 'em
  line_buffer_insert(rs, 9, 3, nl, NULL);

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

  line_buffer_free(rs);
}

static void
test_line_buffer_newlines_only (void) {
  line_buffer_t* rs = line_buffer_init(NULL);
  line_buffer_refresh(rs);

  char nl[1];
  nl[0] = '\n';
  nl[1] = '\0';
  line_buffer_insert(rs, 0, 0, nl, NULL);
  line_buffer_insert(rs, 0, 1, nl, NULL);

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

  line_buffer_free(rs);
}

static void
test_line_buffer_get_line (void) {
  char*          raw = "hello\nworld\nthis\nis a line";
  line_buffer_t* rs  = line_buffer_init(raw);
  line_buffer_refresh(rs);

  ok(array_size(rs->line_info) == 4, "has 4 lines");
  ok(rs->num_lines == 4, "num_lines field is correct");

  is(get_line(rs, 0), "hello", "returns the expected line");

  is(get_line(rs, 1), "world", "returns the expected line");

  is(get_line(rs, 2), "this", "returns the expected line");

  is(get_line(rs, 3), "is a line", "returns the expected line");

  // Now we test that it works after modifying the buffer.
  // We'll also simulate the cursor edits

  line_buffer_insert(rs, 5, 0, "x", NULL);

  ok(array_size(rs->line_info) == 4, "has 4 lines");
  ok(rs->num_lines == 4, "num_lines field is correct");

  is(get_line(rs, 0), "hellox", "returns the expected line");

  is(get_line(rs, 1), "world", "returns the expected line");

  is(get_line(rs, 2), "this", "returns the expected line");

  is(get_line(rs, 3), "is a line", "returns the expected line");

  line_buffer_insert(rs, 1, 1, "x", NULL);

  line_buffer_insert(rs, 3, 1, "x", NULL);

  line_buffer_insert(rs, 3, 3, "x", NULL);

  line_buffer_insert(rs, 5, 3, "x", NULL);

  ok(array_size(rs->line_info) == 4, "has 4 lines");
  ok(rs->num_lines == 4, "num_lines field is correct");

  is(get_line(rs, 0), "hellox", "returns the expected line");

  is(get_line(rs, 1), "wxoxrld", "returns the expected line");

  is(get_line(rs, 2), "this", "returns the expected line");

  is(get_line(rs, 3), "is xax line", "returns the expected line");

  line_buffer_delete(rs, 5, 0, NULL);

  line_buffer_delete(rs, 2, 1, NULL);

  line_buffer_delete(rs, 4, 3, NULL);

  line_buffer_insert(rs, 3, 3, "x", NULL);

  ok(array_size(rs->line_info) == 4, "has 4 lines");
  ok(rs->num_lines == 4, "num_lines field is correct");

  is(get_line(rs, 0), "hello", "returns the expected line");

  is(get_line(rs, 1), "wxxrld", "returns the expected line");

  is(get_line(rs, 2), "this", "returns the expected line");

  is(get_line(rs, 3), "is xxx line", "returns the expected line");

  line_buffer_delete(rs, -1, 1, NULL);

  ok(array_size(rs->line_info) == 3, "has 3 lines");
  ok(rs->num_lines == 3, "num_lines field is correct");

  is(get_line(rs, 0), "hellowxxrld", "returns the expected line");

  is(get_line(rs, 1), "this", "returns the expected line");

  is(get_line(rs, 2), "is xxx line", "returns the expected line");

  line_buffer_free(rs);
}

static void
test_line_buffer_undo (void) {
  line_buffer_t* rs = line_buffer_init(NULL);
  line_buffer_refresh(rs);

  cursor_t *c1, *c2, *c3, *c4, *c5, *c6, *c7, *c8, *c9;
  c1 = create_test_cursor(0, 0);
  c2 = create_test_cursor(1, 0);
  c3 = create_test_cursor(2, 0);
  // In the editor this would be 2:0 but we want to prove that c4 gets
  // persisted.
  c4 = create_test_cursor(3, 0);

  line_buffer_insert(rs, 0, 0, "h", (void*)c1);
  line_buffer_insert(rs, 1, 0, "e", (void*)c2);
  line_buffer_insert(rs, 2, 0, "l", (void*)c3);
  line_buffer_delete(rs, 2, 0, (void*)c4);

  cursor_t* curs = (cursor_t*)line_buffer_undo(rs);
  ok(curs->x == 3, "stored cursor is the correct value");
  ok(curs->y == 0, "stored cursor is the correct value");
  is(get_line(rs, 0), "hel", "undo reverts the delete char");

  curs = (cursor_t*)line_buffer_undo(rs);
  ok(curs->x == 0, "stored cursor is now zero'd");
  ok(curs->y == 0, "stored cursor is now zero'd");
  is(get_line(rs, 0), "", "second undo reverts the entire word");

  free(c1);
  free(c2);
  free(c3);
  free(c4);

  c1 = create_test_cursor(0, 0);
  c2 = create_test_cursor(1, 0);
  c3 = create_test_cursor(2, 0);
  c4 = create_test_cursor(3, 0);
  c5 = create_test_cursor(4, 0);
  c6 = create_test_cursor(0, 1);
  c7 = create_test_cursor(1, 1);
  c8 = create_test_cursor(2, 1);
  c9 = create_test_cursor(5, 0);

  line_buffer_insert(rs, 0, 0, "o", (void*)c1);
  line_buffer_insert(rs, 1, 0, "n", (void*)c2);
  line_buffer_insert(rs, 2, 0, "e", (void*)c3);
  piece_table_break(rs->pt);  // Simulate what we do when char is delimiter
  line_buffer_insert(rs, 3, 0, " ", (void*)c4);
  line_buffer_insert(rs, 4, 0, "x", (void*)c5);
  line_buffer_delete(rs, 4, 0, (void*)c9);
  line_buffer_insert(rs, 4, 0, "\n", (void*)c5);
  line_buffer_insert(rs, 0, 1, "t", (void*)c6);
  line_buffer_insert(rs, 1, 1, "w", (void*)c7);
  line_buffer_insert(rs, 2, 1, "o", (void*)c8);

  ok(rs->num_lines == 2, "has two lines");
  is(get_line(rs, 0), "one ", "first line correct");
  is(get_line(rs, 1), "two", "second line correct");

  cursor_t* meta_c = (cursor_t*)line_buffer_undo(rs);
  ok(meta_c->y == 0, "sets cursor back to previous line");
  ok(meta_c->x == 4, "sets cursor back to previous line");
  ok(rs->num_lines == 1, "has one line now");
  is(get_line(rs, 0), "one ", "is the state before the newline");

  meta_c = (cursor_t*)line_buffer_undo(rs);
  ok(meta_c->y == 0, "sets the cursor after the un-deleted char");
  ok(meta_c->x == 5, "sets the cursor after the un-deleted char");
  ok(rs->num_lines == 1, "still has one line");
  is(get_line(rs, 0), "one x", "reverts the delete");

  meta_c = (cursor_t*)line_buffer_undo(rs);
  ok(meta_c->y == 0, "sets the cursor back to right after the first word");
  ok(meta_c->x == 3, "sets the cursor back to right after the first word");
  ok(rs->num_lines == 1, "still has one line");
  is(get_line(rs, 0), "one", "is just the first word now");

  meta_c = (cursor_t*)line_buffer_undo(rs);
  ok(meta_c->y == 0, "sets the cursor back to cell zero");
  ok(meta_c->x == 0, "sets the cursor back to cell zero");
  ok(rs->num_lines == 1, "still one line");
  is(get_line(rs, 0), "", "no content doe");

  free(c1);
  free(c2);
  free(c3);
  free(c4);
  free(c5);
  free(c6);
  free(c7);
  free(c8);
  free(c9);

  line_buffer_free(rs);
}

static void
test_line_buffer_undo_delete_blocks (void) {
  line_buffer_t* rs = line_buffer_init("hello world");
  line_buffer_refresh(rs);

  cursor_t *c1, *c2, *c3, *c4, *c5, *c6;
  c1 = create_test_cursor(11, 0);
  c2 = create_test_cursor(10, 0);
  c3 = create_test_cursor(9, 0);
  c4 = create_test_cursor(5, 0);
  c5 = create_test_cursor(4, 0);
  c6 = create_test_cursor(3, 0);

  line_buffer_delete(rs, 10, 0, (void*)c1);
  line_buffer_delete(rs, 9, 0, (void*)c2);
  line_buffer_delete(rs, 8, 0, (void*)c3);

  line_buffer_delete(rs, 4, 0, (void*)c4);
  line_buffer_delete(rs, 3, 0, (void*)c5);
  line_buffer_delete(rs, 2, 0, (void*)c6);

  cursor_t* curs = (cursor_t*)line_buffer_undo(rs);
  ok(curs->x == 5, "stored cursor is the correct value");
  ok(curs->y == 0, "stored cursor is the correct value");
  is(get_line(rs, 0), "hello wo", "targets only the last delete block");

  curs = (cursor_t*)line_buffer_undo(rs);
  ok(curs->x == 11, "stored cursor is the correct value");
  ok(curs->y == 0, "stored cursor is the correct value");
  is(
    get_line(rs, 0),
    "hello world",
    "targets the first delete block - undo stack automatically honors index "
    "breaks"
  );

  line_buffer_delete(rs, 10, 0, (void*)c1);
  line_buffer_delete(rs, 8, 0, (void*)c3);

  curs = (cursor_t*)line_buffer_undo(rs);
  ok(curs->x == 9, "stored cursor is the correct value");
  ok(curs->y == 0, "stored cursor is the correct value");
  is(get_line(rs, 0), "hello worl", "undo reverts the deleted block");

  curs = (cursor_t*)line_buffer_undo(rs);
  ok(curs->x == 11, "stored cursor is the correct value");
  ok(curs->y == 0, "stored cursor is the correct value");
  is(get_line(rs, 0), "hello world", "honors index breaks even within a closed block");
}

static void
test_line_buffer_undo_breaks (void) {
  line_buffer_t* rs = line_buffer_init(NULL);
  line_buffer_refresh(rs);

  cursor_t *c1, *c2, *c3, *c4, *c5, *c6;

  c1 = create_test_cursor(0, 0);
  c2 = create_test_cursor(1, 0);
  c3 = create_test_cursor(2, 0);
  c4 = create_test_cursor(3, 0);
  c5 = create_test_cursor(4, 0);
  c6 = create_test_cursor(5, 0);

  line_buffer_insert(rs, 0, 0, "x", (void*)c1);
  line_buffer_insert(rs, 1, 0, "x", (void*)c2);
  piece_table_break(rs->pt);
  line_buffer_insert(rs, 2, 0, " ", (void*)c3);
  line_buffer_insert(rs, 3, 0, "x", (void*)c4);
  line_buffer_insert(rs, 4, 0, "x", (void*)c5);
  piece_table_break(rs->pt);
  line_buffer_insert(rs, 5, 0, " ", (void*)c6);

  ok(rs->num_lines == 1, "has one line");
  is(get_line(rs, 0), "xx xx ", "has expected starting state");

  cursor_t* meta_c;

  meta_c = (cursor_t*)line_buffer_undo(rs);
  is(get_line(rs, 0), "xx xx", "undo goes to the end of the last unit");
  ok(meta_c->y == 0, "metadata cursor correct");
  ok(meta_c->x == 5, "metadata cursor correct");
  ok(rs->num_lines == 1, "still 1 line");

  meta_c = (cursor_t*)line_buffer_undo(rs);
  is(get_line(rs, 0), "xx", "undo goes to the end of the last unit");
  ok(meta_c->y == 0, "metadata cursor correct");
  ok(meta_c->x == 2, "metadata cursor correct");
  ok(rs->num_lines == 1, "still 1 line");

  meta_c = (cursor_t*)line_buffer_undo(rs);
  ok(meta_c->y == 0, "metadata cursor correct");
  ok(meta_c->x == 0, "metadata cursor correct");
  ok(rs->num_lines == 1, "still 1 line");
  is(get_line(rs, 0), "", "undo goes to the end of the last unit i.e. empty line buffer");

  meta_c = (cursor_t*)line_buffer_undo(rs);
  ok(meta_c == NULL, "metadata is NULL because we're at a terminal state (undo stack is empty)");
  ok(rs->num_lines == 1, "still 1 line");
  is(get_line(rs, 0), "", "no-op because we're done");
}

static void
test_line_buffer_undo_multiple_delimiters (void) {
  line_buffer_t* rs = line_buffer_init(NULL);
  line_buffer_refresh(rs);

  cursor_t *c1, *c2, *c3, *c4, *c5, *c6, *c7, *c8, *c9;
}

static void
test_line_buffer_type_then_delete (void) {
  line_buffer_t* rs = line_buffer_init(NULL);
  line_buffer_refresh(rs);
  line_buffer_insert(rs, 0, 0, "d", NULL);
  line_buffer_insert(rs, 1, 0, "d", NULL);
  line_buffer_insert(rs, 2, 0, "d", NULL);
  line_buffer_delete(rs, 2, 0, NULL);

  ok(array_size(rs->line_info) == 1, "has 1 line");
  ok(rs->num_lines == 1, "num_lines field is correct");
  is(get_line(rs, 0), "dd", "correct end state");
}

static void
test_line_buffer_type_then_delete_earlier_pos (void) {
  // In this scenario, the user types a few characters, then moves the cursor
  // back 1 space and deletes.
  line_buffer_t* rs = line_buffer_init(NULL);
  line_buffer_refresh(rs);
  line_buffer_insert(rs, 0, 0, "d", NULL);
  line_buffer_insert(rs, 1, 0, "d", NULL);
  line_buffer_insert(rs, 2, 0, "d", NULL);
  line_buffer_delete(rs, 1, 0, NULL);
  line_buffer_delete(rs, 0, 0, NULL);

  ok(array_size(rs->line_info) == 1, "has 1 line");
  ok(rs->num_lines == 1, "num_lines field is correct");
  is(get_line(rs, 0), "d", "correct end state");
}

static void
test_line_buffer_insert_line_on_first (void) {
  line_buffer_t* rs = line_buffer_init(NULL);
  line_buffer_refresh(rs);
  line_buffer_insert(rs, 0, 0, "hello\n", NULL);
  line_buffer_insert(rs, 0, 0, "hello\n", NULL);

  ok(array_size(rs->line_info) == 3, "has 2 lines");
  ok(rs->num_lines == 3, "num_lines field is correct");
  is(get_line(rs, 0), "hello", "correct end state");
  is(get_line(rs, 1), "hello", "correct end state");
  is(get_line(rs, 2), "", "correct end state");
}

void
run_line_buffer_tests (void) {
  test_line_buffer();
  test_line_buffer_newline();
  test_line_buffer_newlines_only();
  test_line_buffer_get_line();
  test_line_buffer_undo();
  test_line_buffer_undo_delete_blocks();
  test_line_buffer_undo_breaks();
  test_line_buffer_undo_multiple_delimiters();
  // The following tests are real scenarios translated to unit tests
  test_line_buffer_type_then_delete();
  test_line_buffer_type_then_delete_earlier_pos();
  test_line_buffer_insert_line_on_first();
}
