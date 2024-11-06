#ifndef WINDOW_H
#define WINDOW_H

#include "libutil/libutil.h"

#define DEFAULT_LNPAD 3

#define REAL_RENDER_X(render_x) \
  (render_x + (line_pad > 0 ? line_pad : DEFAULT_LNPAD) + 1)

typedef struct {
  // Num rows in the window
  unsigned int rows;
  // Num cols in the window
  unsigned int cols;
} window_t;

void window_clear(void);
void window_refresh(void);
void window_set_sbar_msg(const char* fmt, ...);
void window_set_cbar_msg(const char* fmt, ...);
void window_draw_rows(buffer_t* buf);
void window_scroll(void);

#endif /* WINDOW_H */
