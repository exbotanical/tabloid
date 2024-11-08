#ifndef WINDOW_H
#define WINDOW_H

#include "libutil/libutil.h"

#define DEFAULT_LNPAD 3

typedef struct {
  // Num rows in the window
  unsigned int rows;
  // Num cols in the window
  unsigned int cols;
} window_t;

void window_clear(void);
void window_refresh(void);
void window_scroll(void);
void window_set_status_bar_msg(const char* fmt, ...);
void window_set_command_bar_msg(const char* fmt, ...);
void window_draw_rows(buffer_t* buf);

#endif /* WINDOW_H */
