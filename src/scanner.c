#include "scanner.h"

void
scanner_init (scanner_t* self, const char* source) {
  self->start   = source;
  self->current = source;
  self->buf     = source;
  self->pos     = 0;
}

char
scanner_next (scanner_t* self) {
  const char next = scanner_peek(self);
  if (next != '\0') {
    self->pos++;
  }

  return next;
}

char
scanner_peek (scanner_t* self) {
  return *(self->buf + self->pos);
}

bool
scanner_consume_if (scanner_t* self, bool (*predicate)(char)) {
  char c;
  if ((c = scanner_peek(self) != '\0')) {
    if (predicate(c)) {
      (void)scanner_next(self);
      return true;
    }
    return false;
  }

  return false;
}

bool
scanner_consume_if_eq (scanner_t* self, char eq_to) {
  char c;
  if ((c = scanner_peek(self)) != '\0') {
    if (c == eq_to) {
      (void)scanner_next(self);
      return true;
    }
    return false;
  }

  return false;
}
