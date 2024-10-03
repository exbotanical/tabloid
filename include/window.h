#ifndef WINDOW_H
#define WINDOW_H

typedef struct {
  // Num rows in the window
  unsigned int rows;
  // Num cols in the window
  unsigned int cols;
} window_t;

void window_clear(void);
void window_refresh(void);
void window_set_status_bar_msg(const char* fmt, ...);

#endif /* WINDOW_H */
