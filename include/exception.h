#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>

static noreturn void
panic (const char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  fprintf(stderr, fmt, va);
  va_end(va);

  exit(EXIT_FAILURE);
}

#endif /* EXCEPTION_H */
