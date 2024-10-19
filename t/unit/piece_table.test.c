#include "piece_table.h"

#include <stdlib.h>

#include "tests.h"

void
test_piece_table (void) {
#define buffer_reset() memset(buffer, 0, 1000)
  char* buffer      = malloc(1000);

  piece_table_t* pt = piece_table_init();
  piece_table_setup(pt, "hello world");  // TODO: null piece test
  piece_table_render(pt, 0, pt->seq_length, buffer);
  is(buffer, "hello world", "renders the correct initial string");
  buffer_reset();

  piece_table_insert(pt, 3, "goodbye");
  piece_table_render(pt, 0, pt->seq_length, buffer);
  is(buffer, "helgoodbyelo world", "renders the correct string after inserting");
  buffer_reset();

  piece_table_insert(pt, 6, "xx");
  piece_table_render(pt, 0, pt->seq_length, buffer);
  is(buffer, "helgooxxdbyelo world", "renders the correct string after inserting again");
  buffer_reset();

  piece_table_delete(pt, 3, 9);
  piece_table_render(pt, 0, pt->seq_length, buffer);
  is(buffer, "hello world", "renders the correct string after deleting some text");
  buffer_reset();

  piece_table_delete(pt, 0, 6);
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

  piece_table_insert(pt, 5, "   xx");
  piece_table_render(pt, 0, pt->seq_length, buffer);
  is(buffer, "world   xx", "inserts more text");
  buffer_reset();

  piece_table_insert(pt, 5, "   yy");
  piece_table_render(pt, 0, pt->seq_length, buffer);
  is(buffer, "world   yy   xx", "inserts even more text");
}

void
run_piece_table_tests (void) {
  test_piece_table();
}
