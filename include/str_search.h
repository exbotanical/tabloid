#ifndef STR_SEARCH_H
#define STR_SEARCH_H

#include <stddef.h>
#include <sys/types.h>

typedef struct {
  int    bad_char_skip[256];
  char  *pattern;
  size_t pattern_len;
  int   *good_suffix_skip;
} string_finder_t;

/* Implements Boyer-Moore search */
void string_finder_init(string_finder_t *self, char *pattern);
int  string_finder_next(string_finder_t *self, char *text, unsigned int find_n);

#endif /* STR_SEARCH_H */
