#include <assert.h>

#include "const.h"
#include "cursor.h"
#include "editor.h"
#include "keypress.h"
#include "tests.h"
#include "window.h"

#define RESET_BUFFERS() \
  buffer_free(buf);     \
  buf = buffer_init(NULL)

static char *file_buffer;

static void
setup (void) {
  // TODO: can we use editor_init somehow? maybe mock get_win
  editor.line_ed.r                = line_buffer_init(NULL);
  editor.line_ed.curs             = DEFAULT_CURSOR_STATE;
  editor.win.cols                 = 0;
  editor.win.rows                 = 0;
  editor.filepath                 = NULL;

  editor.conf.ln_prefix           = DEFAULT_LINE_PREFIX;

  line_pad                        = DEFAULT_LNPAD;

  editor.win.rows                 = 40;
  editor.win.cols                 = 50;

  editor.s_bar.left_component[0]  = '\0';
  editor.s_bar.right_component[0] = '\0';
  editor.mode                     = EDIT_MODE;

  FILE *fd                        = fopen("./t/fixtures/file.txt", "r");
  if (!fd) {
    perror("fopen");
    exit(1);
  }

  file_buffer         = malloc(68300);
  size_t          sz  = sizeof(file_buffer);
  read_all_result ret = read_all(fd, &file_buffer, &sz);
  assert(ret == READ_ALL_OK);
  line_buffer_insert(editor.line_ed.r, editor.line_ed.curs.x, editor.line_ed.curs.y, file_buffer, NULL);
}

static void
teardown (void) {
  free(editor.line_ed.r);
  free(file_buffer);
}

static void
test_basic_draw_status_bar (void) {
  ok(editor.line_ed.r->num_lines == 38, "sanity check");
  ok(editor.line_ed.curs.x == 0, "sanity check");
  ok(editor.line_ed.curs.y == 0, "sanity check");

  buffer_t *buf = buffer_init(NULL);

  window_draw_status_bar(buf);
  is(buffer_state(buf), ESC_SEQ_INVERT_COLOR " | EDIT | [No Name]                 | Ln 1, Col 1 " ESC_SEQ_NORM_COLOR, "");

  RESET_BUFFERS();

  editor.mode = COMMAND_MODE;
  window_draw_status_bar(buf);
  is(buffer_state(buf), ESC_SEQ_INVERT_COLOR " | COMMAND | [No Name]              | Ln 1, Col 1 " ESC_SEQ_NORM_COLOR, "");

  RESET_BUFFERS();

  editor.mode = EDIT_MODE;
  cursor_move_down(&editor.line_ed);
  cursor_move_end(&editor.line_ed);
  window_draw_status_bar(buf);
  is(buffer_state(buf), ESC_SEQ_INVERT_COLOR " | EDIT | [No Name]                 | Ln 2, Col 5 " ESC_SEQ_NORM_COLOR, "");

  RESET_BUFFERS();

  editor.filepath = "/glenn/greenwald.txt";
  window_draw_status_bar(buf);
  // TODO: filename only, truncate+ellipses if necessary
  is(buffer_state(buf), ESC_SEQ_INVERT_COLOR " | EDIT | /glenn/greenwald.txt      | Ln 2, Col 5 " ESC_SEQ_NORM_COLOR, "");

  buffer_free(buf);
}

void
run_status_bar_tests (void) {
  void (*functions[])() = {
    test_basic_draw_status_bar,
  };

  for (unsigned int i = 0; i < sizeof(functions) / sizeof(functions[0]); i++) {
    setup();
    functions[i]();
    teardown();
  }
}
