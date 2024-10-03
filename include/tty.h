#ifndef TTY_H
#define TTY_H

#include <termios.h>

typedef struct {
  struct termios og_tty;
} tty_t;

void tty_enable_raw_mode(void);
void tty_disable_raw_mode(void);
int  tty_get_window_sz(unsigned int* rows, unsigned int* cols);

#endif /* TTY_H */
