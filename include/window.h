#ifndef WINDOW_H
#define WINDOW_H

#define DEFAULT_LNPAD 3

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

#endif /* WINDOW_H */
