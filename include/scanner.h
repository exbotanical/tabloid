#ifndef SCANNER_H
#define SCANNER_H

#include <stdbool.h>
#include <sys/types.h>

typedef struct {
  const char* start;
  const char* current;
  ssize_t     pos;
  const char* buf;
} scanner_t;

void scanner_init(scanner_t* self, const char* source);
char scanner_next(scanner_t* self);
char scanner_peek(scanner_t* self);
bool scanner_consume_if_eq(scanner_t* self, char eq_to);

#endif /* SCANNER_H */
