#ifndef EDITOR_H
#define EDITOR_H

#include "config.h"
#include "cursor.h"
#include "file.h"
#include "line_buffer.h"
#include "tty.h"
#include "window.h"

typedef struct {
  /**
   * Raw line state
   */
  char*        raw;
  /**
   * Raw line state size/length
   */
  unsigned int raw_sz;
  /**
   * Actual chars to render on the screen
   */
  char*        render_buf;
  /**
   * Size of the render buffer
   */
  unsigned int render_buf_sz;
} line_buffer_t;

typedef struct {
  // Number of lines in the entire editor buffer
  unsigned int   num_lines;
  // Actual line buffers
  line_buffer_t* lines;
} buffer_state_t;

typedef struct {
  // TODO:
  char msg[64];
} s_bar_state_t;

typedef struct {
  // TODO:
  char msg[64];
} c_bar_state_t;

typedef struct {
  cursor_t       curs;
  window_t       win;
  buffer_state_t buf;
  tty_t          tty;
  config_t       conf;
  file_handle_t  fs;
  s_bar_state_t  s_bar;
  c_bar_state_t  c_bar;
  render_state_t r;
} editor_t;

void editor_init(void);
void editor_open(const char* filename);
void editor_insert_char(int c);
void editor_del_char(void);
void editor_insert_newline(void);
void editor_insert_row(int at, char* s, size_t len);

#endif /* EDITOR_H */
