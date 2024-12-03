#include "command_bar.h"

#include "const.h"
#include "cursor.h"
#include "keypress.h"
#include "tests.h"
#include "window.h"

#define RESET_BUFFERS() \
  buffer_free(buf);     \
  buf = buffer_init(NULL)

size_t
tty_get_window_size (size_t *rows, size_t *cols) {
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

static void
test_basic_draw_command_bar (void) {
  ok(editor.line_ed.r->num_lines == 38, "sanity check");
  ok(editor.line_ed.curs.x == 0, "sanity check");
  ok(editor.line_ed.curs.y == 0, "sanity check");

  editor.mode   = COMMAND_MODE;
  editor.cmode  = CB_INPUT;
  buffer_t *buf = buffer_init(NULL);

  CALL_N_TIMES(3, line_editor_insert_char(&editor.c_bar, 'x'));

  window_draw_command_bar(buf);

  is(buffer_state(buf), COMMAND_BAR_PREFIX "xxx" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR, "draws the command bar");

  RESET_BUFFERS();

  command_bar_clear(&editor.c_bar);
  window_draw_command_bar(buf);

  is(buffer_state(buf), COMMAND_BAR_PREFIX " " ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR, "empty command bar after clear");

  RESET_BUFFERS();

  CALL_N_TIMES(3, line_editor_insert_char(&editor.c_bar, 'x'));
  line_editor_delete_line_before_x(&editor.c_bar);
  window_draw_command_bar(buf);

  is(buffer_state(buf), COMMAND_BAR_PREFIX " " ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR, "empty command bar after delete line");

  RESET_BUFFERS();

  mode_chmod(COMMAND_MODE);
  CALL_N_TIMES(3, line_editor_insert_char(&editor.c_bar, 'x'));
  window_draw_command_bar(buf);

  is(buffer_state(buf), COMMAND_BAR_PREFIX "xxx" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR, "command bar has prefix and text in command mode");

  RESET_BUFFERS();

  command_bar_set_message_mode(&editor.c_bar, "test");
  window_draw_command_bar(buf);
  is(buffer_state(buf), "test                                              " ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR, "command bar has message in command.message mode");

  buffer_free(buf);
}

void
run_command_bar_tests (void) {
  void (*functions[])() = {
    test_basic_draw_command_bar,
  };

  for (size_t i = 0; i < sizeof(functions) / sizeof(functions[0]); i++) {
    setup();
    functions[i]();
    teardown();
  }
}
