#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <stdnoreturn.h>

noreturn void panic(const char *fmt, ...);

#endif /* EXCEPTION_H */
