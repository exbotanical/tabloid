#ifndef MODE_H
#define MODE_H

typedef enum {
  EDIT_MODE = 1,
  COMMAND_MODE,
} editor_mode_t;

void mode_chmod(editor_mode_t next);
void mode_status_bar(void);

#endif /* MODE_H */
