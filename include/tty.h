#ifndef TTY_H
#define TTY_H

#include <stddef.h>
#include <sys/types.h>
#include <termios.h>

typedef struct {
  struct termios og_tty;
} tty_t;

void                         tty_enable_raw_mode(void);
void                         tty_disable_raw_mode(void);
__attribute__((weak)) size_t tty_get_window_size(size_t* rows, size_t* cols);
void                         tty_clear(void);

#endif /* TTY_H */
