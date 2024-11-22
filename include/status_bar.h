#ifndef STATUS_BAR_H
#define STATUS_BAR_H

typedef struct {
  // TODO: size guards
  char left_component[64];
  char right_component[64];
} s_bar_state_t;

void status_bar_set_left_component_msg(const char* fmt, ...);
void status_bar_set_right_component_msg(const char* fmt, ...);

#endif /* STATUS_BAR_H */
