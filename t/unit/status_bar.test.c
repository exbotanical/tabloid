#include "status_bar.h"

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

unsigned int
tty_get_window_size (unsigned int *rows, unsigned int *cols) {
  *rows = 40;
  *cols = 50;
  return 0;
}

static void
setup (void) {
  editor_init(&editor);
  editor_open("./t/fixtures/file.txt");
}

static void
teardown (void) {
  editor_free(&editor);
}

/* clang-format off */
static void
test_basic_draw_status_bar (void) {
  ok(editor.line_ed.r->num_lines == 38, "sanity check");
  ok(editor.line_ed.curs.x == 0, "sanity check");
  ok(editor.line_ed.curs.y == 0, "sanity check");

  buffer_t *buf = buffer_init(NULL);

  window_draw_status_bar(buf);
  is(
    buffer_state(buf),
    ESC_SEQ_INVERT_COLOR " | EDIT | file.txt                  | Ln 1, Col 1 " ESC_SEQ_NORM_COLOR,
    "draws the status bar with edit mode"
  );

  RESET_BUFFERS();

  editor.mode = COMMAND_MODE;
  window_draw_status_bar(buf);
  is(
    buffer_state(buf),
    ESC_SEQ_INVERT_COLOR " | COMMAND | file.txt               | Ln 1, Col 1 " ESC_SEQ_NORM_COLOR,
    "draws the status bar with command mode"
  );

  RESET_BUFFERS();

  editor.mode = EDIT_MODE;
  cursor_move_down(&editor.line_ed);
  cursor_move_end(&editor.line_ed);
  window_draw_status_bar(buf);
  is(
    buffer_state(buf),
    ESC_SEQ_INVERT_COLOR " | EDIT | file.txt                  | Ln 2, Col 5 " ESC_SEQ_NORM_COLOR,
    "draws the status bar correctly"
  );

  RESET_BUFFERS();

  line_editor_insert_char(&editor.line_ed, 'x');
  window_draw_status_bar(buf);

  is(
    buffer_state(buf),
    ESC_SEQ_INVERT_COLOR " | EDIT | file.txt*                 | Ln 2, Col 6 " ESC_SEQ_NORM_COLOR,
    "draws the status bar correctly"
  );

  RESET_BUFFERS();

  // TODO: filename truncate+ellipses if necessary

  buffer_free(buf);
}

/* clang-format on */

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
