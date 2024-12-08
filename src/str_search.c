#include "str_search.h"

#include <alloca.h>
#include <stdbool.h>
#include <string.h>

#include "calc.h"
#include "libutil/libutil.h"

static bool
has_prefix (char *s, char *prefix) {
  size_t s_len      = strlen(s);
  size_t prefix_len = strlen(prefix);
  char  *sub_s      = s_substr(s, 0, prefix_len, false);

  return s_len >= prefix_len && s_equals(sub_s ? sub_s : "", prefix);
}

static int
longest_common_suffix (char *a, char *b) {
  int    i     = 0;
  size_t a_len = strlen(a);
  size_t b_len = strlen(b);

  for (; a_len && i < b_len; i++) {
    if (a[a_len - 1 - i] != b[b_len - 1 - i]) {
      break;
    }
  }

  return i;
}

void
string_finder_init (string_finder_t *self, char *pattern) {
  self->pattern_len = strlen(pattern);
  self->pattern     = pattern;

  int last          = self->pattern_len - 1;

  self->bad_char_skip[256];
  self->good_suffix_skip = alloca(self->pattern_len + 2);

  for (int i = 0; i < 256; i++) {
    self->bad_char_skip[i] = self->pattern_len;
  }

  for (int i = 0; i < last; i++) {
    self->bad_char_skip[self->pattern[i]] = last - i;
  }

  int last_prefix = last;
  for (int i = last; i >= 0; i--) {
    char *prefix = s_substr(self->pattern, i + 1, self->pattern_len - 1, true);

    bool cond    = prefix ? has_prefix(self->pattern, prefix) : has_prefix(self->pattern, "");
    if (cond) {
      last_prefix = i + 1;
    }

    self->good_suffix_skip[i] = last_prefix + last - i;
  }

  for (int i = 0; i < last; i++) {
    char  *suffix     = s_substr(self->pattern, 1, i + 1, false);
    size_t suffix_len = suffix ? longest_common_suffix(self->pattern, suffix) : 0;

    if (self->pattern[i - suffix_len] != self->pattern[last - suffix_len]) {
      self->good_suffix_skip[last - suffix_len] = suffix_len + last - i;
    }
  }
}

int
string_finder_next (string_finder_t *self, char *text, unsigned int find_n) {
  int    i        = self->pattern_len - 1;
  size_t text_len = strlen(text);

  while (i < (int)text_len) {
    int j = self->pattern_len - 1;

    while (j >= 0 && text[i] == self->pattern[j]) {
      i--;
      j--;
    }

    if ((int)j < 0) {
      return i + 1;
    }

    i += max(self->bad_char_skip[text[i]], self->good_suffix_skip[j]);
  }

  return -1;
}
