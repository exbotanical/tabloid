#include "piece_table.h"

#include <stdlib.h>

#include "tests.h"
#include "xmalloc.h"

static void
test_piece_table (void) {
#define buffer_reset() memset(buffer, 0, 1000)
  char* buffer      = xmalloc(1000);

  piece_table_t* pt = piece_table_init();
  piece_table_setup(pt, "hello world");
  piece_table_render(pt, 0, pt->seq_length, buffer);
  is(buffer, "hello world", "renders the correct initial string");
  buffer_reset();

  piece_table_insert(pt, 3, "goodbye", NULL);
  piece_table_render(pt, 0, pt->seq_length, buffer);
  is(buffer, "helgoodbyelo world", "renders the correct string after inserting");
  buffer_reset();

  piece_table_insert(pt, 6, "xx", NULL);
  piece_table_render(pt, 0, pt->seq_length, buffer);
  is(buffer, "helgooxxdbyelo world", "renders the correct string after inserting again");
  buffer_reset();

  piece_table_delete(pt, 3, 9, PT_DELETE, NULL);
  piece_table_render(pt, 0, pt->seq_length, buffer);
  is(buffer, "hello world", "renders the correct string after deleting some text");
  buffer_reset();

  piece_table_delete(pt, 0, 6, PT_DELETE, NULL);
  piece_table_render(pt, 0, pt->seq_length, buffer);
  is(buffer, "world", "renders the correct string after deleting some more text");
  buffer_reset();

  piece_table_undo(pt);
  piece_table_render(pt, 0, pt->seq_length, buffer);
  is(buffer, "hello world", "undos the last change");
  buffer_reset();

  piece_table_redo(pt);
  piece_table_render(pt, 0, pt->seq_length, buffer);
  is(buffer, "world", "redos the last change");
  buffer_reset();

  piece_table_insert(pt, 5, "   xx", NULL);
  piece_table_render(pt, 0, pt->seq_length, buffer);
  is(buffer, "world   xx", "inserts more text");
  buffer_reset();

  piece_table_insert(pt, 5, "   yy", NULL);
  piece_table_render(pt, 0, pt->seq_length, buffer);
  is(buffer, "world   yy   xx", "inserts even more text");

  piece_table_free(pt);
  free(buffer);
}

static void
test_piece_table_no_initial (void) {
#define buffer_reset() memset(buffer, 0, 1000)
  char* buffer      = xmalloc(1000);

  piece_table_t* pt = piece_table_init();
  piece_table_setup(pt, NULL);
  piece_table_render(pt, 0, pt->seq_length, buffer);
  is(buffer, "", "renders an empty initial string");
  memset(buffer, 0, 1000);

  piece_table_insert(pt, 0, "hello", NULL);
  piece_table_render(pt, 0, pt->seq_length, buffer);
  is(buffer, "hello", "renders the appended string");

  piece_table_free(pt);
  free(buffer);
}

static void
test_piece_table_empty_initial (void) {
#define buffer_reset() memset(buffer, 0, 1000)
  char* buffer      = xmalloc(1000);

  piece_table_t* pt = piece_table_init();
  piece_table_setup(pt, "");
  piece_table_render(pt, 0, pt->seq_length, buffer);
  is(buffer, "", "renders an empty initial string");
  memset(buffer, 0, 1000);

  piece_table_insert(pt, 0, "hello", NULL);
  piece_table_render(pt, 0, pt->seq_length, buffer);
  is(buffer, "hello", "renders the appended string");

  piece_table_free(pt);
  free(buffer);
}

static void
test_piece_table_dirty (void) {
  piece_table_t* pt = piece_table_init();
  piece_table_setup(pt, "xxx");

  ok(piece_table_dirty(pt) == false, "");

  piece_table_insert(pt, 0, "hello", NULL);
  ok(piece_table_dirty(pt) == true, "");

  piece_table_undo(pt);
  ok(piece_table_dirty(pt) == false, "");

  piece_table_redo(pt);
  ok(piece_table_dirty(pt) == true, "");

  piece_table_undo(pt);
  ok(piece_table_dirty(pt) == false, "");

  piece_table_delete(pt, 1, 1, PT_DELETE, NULL);
  ok(piece_table_dirty(pt) == true, "");

  piece_table_undo(pt);
  ok(piece_table_dirty(pt) == false, "");

  piece_table_free(pt);
}

void
run_piece_table_tests (void) {
  test_piece_table();
  test_piece_table_no_initial();
  test_piece_table_empty_initial();
  test_piece_table_dirty();
}
