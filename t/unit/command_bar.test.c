#include "const.h"
#include "cursor.h"
#include "keypress.h"
#include "tests.h"
#include "window.h"

#define RESET_BUFFERS() \
  buffer_free(buf);     \
  buf = buffer_init(NULL)

static char  buf[128];
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
  line_buffer_insert(editor.line_ed.r, editor.line_ed.curs.x, editor.line_ed.curs.y, file_buffer, NULL);
}

static void
teardown (void) {
  free(editor.line_ed.r);
  free(file_buffer);
}

static void
test_basic_draw_command_bar (void) {
  ok(editor.line_ed.r->num_lines == 38, "sanity check");
  ok(editor.line_ed.curs.x == 0, "sanity check");
  ok(editor.line_ed.curs.y == 0, "sanity check");

  buffer_t *buf = buffer_init(NULL);

  // window_draw_command_bar(buf);

  buffer_free(buf);
}

void
run_command_bar_tests (void) {
  void (*functions[])() = {
    test_basic_draw_command_bar,
  };

  for (unsigned int i = 0; i < sizeof(functions) / sizeof(functions[0]); i++) {
    setup();
    functions[i]();
    teardown();
  }
}
