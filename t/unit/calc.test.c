#include "calc.h"

#include "tests.h"

static void
test_min (void) {
  ssize_t a   = 1;
  ssize_t b   = 2;

  ssize_t ret = min(a, b);
  ok(ret == 1, "returns the smaller number");
}

static void
test_min_neg (void) {
  ssize_t a   = 1;
  ssize_t b   = -2;

  ssize_t ret = min(a, b);
  ok(ret == -2, "returns the smaller number");
}

static void
test_min_both_neg (void) {
  ssize_t a   = -100;
  ssize_t b   = -20;

  ssize_t ret = min(a, b);
  ok(ret == -100, "returns the smaller number");
}

static void
test_min_eq (void) {
  ssize_t a   = 2;
  ssize_t b   = 2;

  ssize_t ret = min(a, b);
  ok(ret == 2, "returns the equivalent number");
}

static void
test_max (void) {
  ssize_t a   = 1;
  ssize_t b   = 2;

  ssize_t ret = max(a, b);
  ok(ret == 2, "returns the larger number");
}

static void
test_max_neg (void) {
  ssize_t a   = 1;
  ssize_t b   = -2;

  ssize_t ret = max(a, b);
  ok(ret == 1, "returns the larger number");
}

static void
test_max_both_neg (void) {
  ssize_t a   = -100;
  ssize_t b   = -20;

  ssize_t ret = max(a, b);
  ok(ret == -20, "returns the larger number");
}

static void
test_max_eq (void) {
  ssize_t a   = 2;
  ssize_t b   = 2;

  ssize_t ret = max(a, b);
  ok(ret == 2, "returns the equivalent number");
}

void
run_calc_tests (void) {
  test_min();
  test_min_neg();
  test_min_both_neg();
  test_min_eq();

  test_max();
  test_max_neg();
  test_max_both_neg();
  test_max_eq();
}
