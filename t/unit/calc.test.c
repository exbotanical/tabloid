#include "calc.h"

#include "tests.h"

void
test_min (void) {
  int a   = 1;
  int b   = 2;

  int ret = min(a, b);
  ok(ret == 1, "returns the smaller number");
}

void
test_min_neg (void) {
  int a   = 1;
  int b   = -2;

  int ret = min(a, b);
  ok(ret == -2, "returns the smaller number");
}

void
test_min_both_neg (void) {
  int a   = -100;
  int b   = -20;

  int ret = min(a, b);
  ok(ret == -100, "returns the smaller number");
}

void
test_min_eq (void) {
  int a   = 2;
  int b   = 2;

  int ret = min(a, b);
  ok(ret == 2, "returns the equivalent number");
}

void
test_max (void) {
  int a   = 1;
  int b   = 2;

  int ret = max(a, b);
  ok(ret == 2, "returns the larger number");
}

void
test_max_neg (void) {
  int a   = 1;
  int b   = -2;

  int ret = max(a, b);
  ok(ret == 1, "returns the larger number");
}

void
test_max_both_neg (void) {
  int a   = -100;
  int b   = -20;

  int ret = max(a, b);
  ok(ret == -20, "returns the larger number");
}

void
test_max_eq (void) {
  int a   = 2;
  int b   = 2;

  int ret = max(a, b);
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
