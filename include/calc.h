#ifndef CALC_H
#define CALC_H

#include <sys/types.h>

static ssize_t
max (ssize_t a, ssize_t b) {
  __typeof__(a) _a = (a);
  __typeof__(b) _b = (b);
  return _a > _b ? _a : _b;
}

static ssize_t
min (ssize_t a, ssize_t b) {
  __typeof__(a) _a = (a);
  __typeof__(b) _b = (b);
  return _a < _b ? _a : _b;
}

#endif /* CALC_H */
