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

#endif /* WINDOW_H */
