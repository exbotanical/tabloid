#ifndef EDITOR_H
#define EDITOR_H

#include "config.h"
#include "file.h"
#include "line_editor.h"
#include "mode.h"
#include "tty.h"
#include "window.h"

typedef struct {
  // TODO: size guards
  char left_component[64];
  char right_component[64];
} s_bar_state_t;

typedef struct {
  line_buffer_t* buf;
} c_bar_state_t;

// TODO: no more global state
typedef struct {
  window_t      win;
  tty_t         tty;
  config_t      conf;
  s_bar_state_t s_bar;
  c_bar_state_t c_bar;
  editor_mode_t mode;
  line_editor_t line_ed;
  const char*   filepath;
} editor_t;

void editor_init(editor_t* self);
void editor_open(const char* filename);

#endif /* EDITOR_H */
