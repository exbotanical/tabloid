#ifndef EDITOR_H
#define EDITOR_H

#include "config.h"
#include "cursor.h"
#include "file.h"
#include "line_buffer.h"
#include "tty.h"
#include "window.h"

typedef struct {
  // TODO:
  char msg[64];
} s_bar_state_t;

typedef struct {
  // TODO:
  char msg[64];
} c_bar_state_t;

typedef struct {
  cursor_t        curs;
  window_t        win;
  tty_t           tty;
  config_t        conf;
  file_handle_t   fs;
  s_bar_state_t   s_bar;
  c_bar_state_t   c_bar;
  render_state_t* r;
} editor_t;

void editor_init(void);
void editor_open(const char* filename);
void editor_insert_char(int c);
void editor_del_char(void);
void editor_delete_from_anchor(void);
void editor_insert_newline(void);
void editor_insert(char* s);

#endif /* EDITOR_H */
