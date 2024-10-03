#include "exception.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

noreturn void
panic (const char *fmt, ...) {
  va_list va;

  va_start(va, fmt);

  perror("errno message: ");
  fprintf(stderr, fmt, va);

  va_end(va);

  exit(EXIT_FAILURE);
}
