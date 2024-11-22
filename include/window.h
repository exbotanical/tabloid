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

extern inline unsigned int window_get_num_rows(void);
extern inline unsigned int window_get_num_cols(void);

void window_clear(void);
void window_refresh(void);
void window_scroll(void);

void window_draw_rows(buffer_t* buf);
void window_draw_status_bar(buffer_t* buf);
void window_draw_command_bar(buffer_t* buf);

#endif /* WINDOW_H */
