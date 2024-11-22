#include "const.h"
#include "editor.h"
#include "keypress.h"
#include "tests.h"

#define RESET_BUFFERS()   \
  memset(slice, 0, 1024); \
  buffer_free(buf);       \
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
}

static void
teardown (void) {
  free(editor.line_ed.r);
  free(file_buffer);
}

/* clang-format off */
static void
test_editor_open (void) {
  editor_open("./t/fixtures/file.txt");

  buffer_t *buf = buffer_init(NULL);
  window_draw_rows(buf);

  ok(editor.line_ed.r->num_lines == 38, "has 38 lines");
  ok(editor.line_ed.curs.x == 0, "cursor at cell zero");
  ok(editor.line_ed.curs.y == 0, "cursor at cell zero");
  is(
    buffer_state(buf),
    ESC_SEQ_COLOR(3) "  1 " ESC_SEQ_NORM_COLOR ESC_SEQ_BG_COLOR(238) "amateuros                                     "
    ESC_SEQ_NORM_COLOR ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    "  2 bash" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    "  3 bolt" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    "  4 ccan" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    "  5 CINEWORLD-NextJS" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    "  6 clib" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    "  7 coreutils" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    "  8 coroutine" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    "  9 dagger" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 10 dcron" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 11 endlessh" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 12 entr" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 13 Fiwix" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 14 grub2" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 15 knex" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 16 k-os" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 17 libuv" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 18 linux" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 19 minix" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 20 Mirai-Source-Code" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 21 neovim" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 22 nodemon" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 23 node-v0.x-archive" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 24 putlockertv.one" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 25 redis" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 26 SQLite-2.5.0-for-code-reading" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 27 traxxx" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 28 v8" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 29 vim" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 30 vite" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 31 vitest" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 32 vixiecron" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 33 vscode-textbuffer" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 34 xi-editor" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 35 xstate" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 36 xv6-annotated" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 37 xv6-public" ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF
    " 38 " ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF DEFAULT_LINE_PREFIX
    ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF DEFAULT_LINE_PREFIX ESC_SEQ_ERASE_LN_RIGHT_OF_CURSOR CRLF,
    "each line is rendered correctly"
  );

  buffer_free(buf);
}

/* clang-format on */

void
run_file_mgmt_tests (void) {
  void (*functions[])() = {
    // TODO: editor_save tests
    test_editor_open,
  };

  for (unsigned int i = 0; i < sizeof(functions) / sizeof(functions[0]); i++) {
    setup();
    functions[i]();
    teardown();
  }
}
