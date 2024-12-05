#ifndef EDITOR_H
#define EDITOR_H

#include "command_bar.h"
#include "config.h"
#include "file.h"
#include "line_editor.h"
#include "mode.h"
#include "status_bar.h"
#include "tty.h"
#include "window.h"

// TODO: no more global state
// TODO: pointers or no? either way, be consistent.
// Read: https://stackoverflow.com/questions/24452323/whats-the-difference-between-pointer-and-value-in-struct
typedef struct {
  window_t       win;
  tty_t          tty;
  config_t       conf;
  s_bar_state_t  s_bar;
  line_editor_t  c_bar;
  editor_mode_t  mode;
  command_mode_t cmode;
  char           cbar_msg[64];
  line_editor_t  line_ed;
  const char*    filepath;
} editor_t;

void editor_init(editor_t* self);
void editor_free(editor_t* self);
void editor_open(const char* filename);
int  editor_save(const char* filepath);

#endif /* EDITOR_H */
