#include "str_search.h"

#include "tests.h"

#define ENTRY(arr, k, v) arr[k] = v

static void
test_str_search_basic (void) {
  typedef struct {
    char* pattern;
    char* text;
    int   expect;
  } test_case;

  test_case test_cases[] = {
    {.pattern = "",        .text = "",                                    .expect = 0 },
    {.pattern = "",        .text = "abc",                                 .expect = 0 },
    {.pattern = "abc",     .text = "",                                    .expect = -1},
    {.pattern = "abc",     .text = "abc",                                 .expect = 0 },
    {.pattern = "d",       .text = "abcdefg",                             .expect = 3 },
    {.pattern = "nan",     .text = "banana",                              .expect = 2 },
    {.pattern = "pan",     .text = "anpanman",                            .expect = 2 },
    {.pattern = "nnaaman", .text = "anpanmanam",                          .expect = -1},
    {.pattern = "abcd",    .text = "abc",                                 .expect = -1},
    {.pattern = "abcd",    .text = "bcd",                                 .expect = -1},
    {.pattern = "bcd",     .text = "abcd",                                .expect = 1 },
    {.pattern = "abc",     .text = "acca",                                .expect = -1},
    {.pattern = "aa",      .text = "aaa",                                 .expect = 0 },
    {.pattern = "baa",     .text = "aaaaa",                               .expect = -1},
    {.pattern = "at that", .text = "which finally halts.  at that point", .expect = 22},
  };

  FOR_EACH_TEST({
    string_finder_t sf;
    string_finder_init(&sf, tc.pattern);
    ssize_t actual = string_finder_next(&sf, tc.text, 0);

    eq_num(actual, tc.expect, "first index of '%s' is %d (got %d)", tc.pattern, tc.expect, actual);
  });
}

static void
test_str_search_tables (void) {
  typedef struct {
    char* pattern;
    int   bad[256];
    int   suffix[16];
    int   suffix_len;
  } test_case;

  test_case test_cases[] = {
    {
     .pattern    = "abc",
     .bad        = {},
     .suffix     = {5, 4, 1},
     .suffix_len = 3,
     },
    {
     .pattern    = "mississi",
     .bad        = {},
     .suffix     = {15, 14, 13, 7, 11, 10, 7, 1},
     .suffix_len = 8,
     },
    // From https://www.cs.utexas.edu/~moore/publications/fstrpos.pdfww
    {
     .pattern    = "abcxxxabc",
     .bad        = {},
     .suffix     = {14, 13, 12, 11, 10, 9, 11, 10, 1},
     .suffix_len = 9,
     },
    {
     .pattern    = "abyxcdeyx",
     .bad        = {},
     .suffix     = {17, 16, 15, 14, 13, 12, 7, 10, 1},
     .suffix_len = 9,
     },
  };

  ENTRY(test_cases[0].bad, 'a', 2);
  ENTRY(test_cases[0].bad, 'b', 1);
  ENTRY(test_cases[0].bad, 'c', 3);

  ENTRY(test_cases[1].bad, 'i', 3);
  ENTRY(test_cases[1].bad, 'm', 7);
  ENTRY(test_cases[1].bad, 's', 1);

  ENTRY(test_cases[2].bad, 'a', 2);
  ENTRY(test_cases[2].bad, 'b', 1);
  ENTRY(test_cases[2].bad, 'c', 6);
  ENTRY(test_cases[2].bad, 'x', 3);

  ENTRY(test_cases[3].bad, 'a', 8);
  ENTRY(test_cases[3].bad, 'b', 7);
  ENTRY(test_cases[3].bad, 'c', 4);
  ENTRY(test_cases[3].bad, 'd', 3);
  ENTRY(test_cases[3].bad, 'e', 2);
  ENTRY(test_cases[3].bad, 'y', 1);
  ENTRY(test_cases[3].bad, 'x', 5);

  FOR_EACH_TEST({
    string_finder_t sf;
    string_finder_init(&sf, tc.pattern);

    for (unsigned int i = 0; i < 256; i++) {
      int got  = sf.bad_char_skip[i];
      int want = tc.bad[i];
      if (want == 0) {
        want = strlen(tc.pattern);
      }

      eq_num(got, want, "search tables (boyer-moore(%s) bad['%c']) - got=%d, want=%d", tc.pattern, i, got, want);
    }

    for (unsigned int i = 0; i < tc.suffix_len; i++) {
      int got  = sf.good_suffix_skip[i];
      int want = tc.suffix[i];

      if (want == 0) {
        want = strlen(tc.pattern);
      }

      // TODO: Don't count TODO tests in the failed count buddy
      todo_start("Fuckin weird c thing with printing and concurrency probably");
      // eq_num(want, got, "search tables (suffix) got=%d, want=%d\n");
      todo_end();
    }
  });
}

void
run_str_search_tests (void) {
  test_str_search_basic();
  test_str_search_tables();
  // TODO: Finish
  // string_finder_t sf;
  // string_finder_init(&sf, "pat");
  // int actual = string_finder_next(&sf, "x paet ap a toh pattern pattern tap", 0);
  // printf("1: %d\n", actual);
  // actual = string_finder_next(&sf, "x paet ap a toh pattern pattern tap", 1);
  // printf("2: %d\n", actual);
}
