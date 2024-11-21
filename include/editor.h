#ifndef EDITOR_H
#define EDITOR_H

#include "config.h"
#include "cursor.h"
#include "file.h"
#include "line_buffer.h"
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
  cursor_t       curs;
  window_t       win;
  tty_t          tty;
  config_t       conf;
  s_bar_state_t  s_bar;
  c_bar_state_t  c_bar;
  line_buffer_t* r;
  editor_mode_t  mode;
  char*          filepath;
} editor_t;

void editor_init(void);
void editor_open(const char* filename);
void editor_insert_char(int c);
void editor_delete_char(void);
void editor_delete_line_before_x(void);
void editor_insert_newline(void);
void editor_insert(char* s);
void editor_undo(void);
void editor_redo(void);

#endif /* EDITOR_H */
