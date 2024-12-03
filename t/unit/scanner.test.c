#include "scanner.h"

#include "tests.h"

static void
assert_next (scanner_t* self, ssize_t pos, char c) {
  ok(self->pos == pos, "expected pos");
  ok(scanner_peek(self) == c, "expected peek next char");
  ok(scanner_next(self) == c, "expected next char");
}

static void
test_scanner_basic (void) {
  scanner_t scanner;
  scanner_init(&scanner, "text");

  assert_next(&scanner, 0, 't');
  assert_next(&scanner, 1, 'e');
  assert_next(&scanner, 2, 'x');
  assert_next(&scanner, 3, 't');
  assert_next(&scanner, 4, '\0');
}

static void
test_scanner_consume_if_eq (void) {
  scanner_t scanner;
  scanner_init(&scanner, "qxqxqq");

  ok(scanner_consume_if_eq(&scanner, 'q') == true, "expected to consume value");
  ok(scanner_consume_if_eq(&scanner, 'x') == true, "expected to consume value");
  ok(scanner_consume_if_eq(&scanner, 'x') == false, "expected to not consume value");
  ok(scanner_consume_if_eq(&scanner, 'q') == true, "expected to consume value");
  ok(scanner_consume_if_eq(&scanner, 'q') == false, "expected to not consume value");
  ok(scanner_consume_if_eq(&scanner, 'x') == true, "expected to consume value");
  ok(scanner_consume_if_eq(&scanner, 'q') == true, "expected to consume value");
  ok(scanner_consume_if_eq(&scanner, 'q') == true, "expected to consume value");
  ok(scanner_consume_if_eq(&scanner, 'q') == false, "expected to not consume value");
  ok(scanner_consume_if_eq(&scanner, 'q') == false, "expected to not consume value");
}

void
run_scanner_tests (void) {
  test_scanner_basic();
  test_scanner_consume_if_eq();
}
