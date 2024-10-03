#ifndef EDITOR_H
#define EDITOR_H

#include "config.h"
#include "cursor.h"
#include "file.h"
#include "tty.h"
#include "window.h"

typedef struct {
  /**
   * Raw row state
   */
  char*        raw;
  /**
   * Raw row state size/length
   */
  unsigned int raw_sz;
  /**
   * Actual chars to render on the screen
   */
  char*        renderbuf;
  /**
   * Size of the render buffer
   */
  unsigned int renderbuf_sz;
} row_buffer_t;

typedef struct {
  // Number of rows in the entire editor buffer
  unsigned int  num_rows;
  // Actual row buffers
  row_buffer_t* rows;
} buffer_state_t;

typedef struct {
  // TODO:
  char msg[64];
} statusbar_state_t;

typedef struct {
  cursor_t          curs;
  window_t          win;
  buffer_state_t    buf;
  tty_t             tty;
  config_t          config;
  file_handle_t     f;
  statusbar_state_t sbar;
  // ???
  unsigned int      renderx;
} editor_t;

void editor_init(void);
void editor_open(const char* filename);
void editor_insert_char(int c);
void editor_del_char(void);
void editor_insert_newline(void);

#endif /* EDITOR_H */
