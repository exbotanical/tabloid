#ifndef CALC_H
#define CALC_H

static int
max (int a, int b) {
  __typeof__(a) _a = (a);
  __typeof__(b) _b = (b);
  return _a > _b ? _a : _b;
}

static int
min (int a, int b) {
  __typeof__(a) _a = (a);
  __typeof__(b) _b = (b);
  return _a < _b ? _a : _b;
}

#endif /* CALC_H */
