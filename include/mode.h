#ifndef MODE_H
#define MODE_H

typedef enum {
  COMMAND_MODE = 1,
  EDIT_MODE    = 2,
} editor_mode_t;

void mode_chmod(editor_mode_t next);
void mode_status_bar(void);

#endif /* MODE_H */
