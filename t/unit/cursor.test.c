#include <stdlib.h>

#include "tests.h"

void
test_cursor_setup (void) {
  editor.buf.lines = malloc(sizeof(line_buffer_t));
}

void
test_cursor_teardown (void) {
  free(editor.buf.lines);
  editor.buf.lines     = NULL;
  editor.buf.num_lines = 0;
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 0, .y = 0, .render_x = DEFAULT_LNPAD};
}

void
test_cursor_move_down (void) {
  editor_insert_row(0, "hello", 6);
  editor_insert_row(0, "world", 6);
  editor_insert_row(0, "what", 5);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 1, .y = 0, .render_x = DEFAULT_LNPAD};

  ok(editor.curs.x == 1, "sanity check");
  ok(editor.curs.y == 0, "sanity check");
  cursor_move_down();
  ok(editor.curs.x == 1, "does not modify x coord");
  ok(editor.curs.y == 1, "increases y coord by 1");
  cursor_move_down();
  ok(editor.curs.x == 1, "does not modify x coord");
  ok(editor.curs.y == 2, "increases y coord by 1");
}

void
test_cursor_move_down_at_bottom (void) {
  editor_insert_row(0, "hello", 6);
  editor_insert_row(0, "hello", 6);

  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 1, .y = 1, .render_x = DEFAULT_LNPAD};

  ok(editor.curs.x == 1, "sanity check");
  ok(editor.curs.y == 1, "sanity check");
  cursor_move_down();
  ok(editor.curs.x == 1, "does not modify x coord");
  ok(editor.curs.y == 1, "cannot move");
  cursor_move_down();
  ok(editor.curs.x == 1, "does not modify x coord");
  ok(editor.curs.y == 1, "cannot move");
}

void
test_cursor_move_down_one_line (void) {
  editor_insert_row(0, "hello", 6);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 1, .y = 0, .render_x = DEFAULT_LNPAD};

  ok(editor.curs.x == 1, "sanity check");
  ok(editor.curs.y == 0, "sanity check");
  cursor_move_down();
  ok(editor.curs.x == 1, "does not modify x coord");
  ok(editor.curs.y == 0, "cannot move");
  cursor_move_down();
  ok(editor.curs.x == 1, "does not modify x coord");
  ok(editor.curs.y == 0, "cannot move");
}

void
test_cursor_move_up (void) {
  editor_insert_row(0, "hello", 6);
  editor_insert_row(0, "world", 6);
  editor_insert_row(0, "what", 5);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 1, .y = 2, .render_x = DEFAULT_LNPAD};

  ok(editor.curs.x == 1, "sanity check");
  ok(editor.curs.y == 2, "sanity check");
  cursor_move_up();
  ok(editor.curs.x == 1, "does not modify x coord");
  ok(editor.curs.y == 1, "decreases y coord by 1");
  cursor_move_up();
  ok(editor.curs.x == 1, "does not modify x coord");
  ok(editor.curs.y == 0, "decreases y coord by 1");
}

void
test_cursor_move_up_at_top (void) {
  editor_insert_row(0, "hello", 6);
  editor_insert_row(0, "world", 6);
  editor_insert_row(0, "what", 5);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 1, .y = 0, .render_x = DEFAULT_LNPAD};

  ok(editor.curs.x == 1, "sanity check");
  ok(editor.curs.y == 0, "sanity check");
  cursor_move_up();
  ok(editor.curs.x == 1, "does not modify x coord");
  ok(editor.curs.y == 0, "decreases y coord by 1");
  cursor_move_up();
  ok(editor.curs.x == 1, "does not modify x coord");
  ok(editor.curs.y == 0, "decreases y coord by 1");
}

void
test_cursor_move_up_one_line (void) {
  editor_insert_row(0, "what", 5);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 1, .y = 0, .render_x = DEFAULT_LNPAD};

  ok(editor.curs.x == 1, "sanity check");
  ok(editor.curs.y == 0, "sanity check");
  cursor_move_up();
  ok(editor.curs.x == 1, "does not modify x coord");
  ok(editor.curs.y == 0, "decreases y coord by 1");
  cursor_move_up();
  ok(editor.curs.x == 1, "does not modify x coord");
  ok(editor.curs.y == 0, "decreases y coord by 1");
}

void
test_cursor_move_left (void) {
  editor_insert_row(0, "hello", 6);
  editor_insert_row(0, "world", 6);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 2, .y = 0, .render_x = DEFAULT_LNPAD};

  ok(editor.curs.x == 2, "sanity check");
  ok(editor.curs.y == 0, "sanity check");
  cursor_move_left();
  ok(editor.curs.x == 1, "moves left");
  ok(editor.curs.y == 0, "does not move the y coord");
  cursor_move_left();
  ok(editor.curs.x == 0, "moves left");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_left_at_begin_of_line (void) {
  editor_insert_row(0, "hello", 6);
  editor_insert_row(0, "world", 6);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 0, .y = 1, .render_x = DEFAULT_LNPAD};

  ok(editor.curs.x == 0, "sanity check");
  ok(editor.curs.y == 1, "sanity check");
  cursor_move_left();
  ok(editor.curs.x == 6, "moves to the end");
  ok(editor.curs.y == 0, "moves up");
  cursor_move_left();
  ok(editor.curs.x == 5, "moves left");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_left_at_begin_of_first_line (void) {
  editor_insert_row(0, "hello", 6);
  editor_insert_row(0, "world", 6);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 0, .y = 0, .render_x = DEFAULT_LNPAD};

  ok(editor.curs.x == 0, "sanity check");
  ok(editor.curs.y == 0, "sanity check");
  cursor_move_left();
  ok(editor.curs.x == 0, "can't move left");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_right (void) {
  editor_insert_row(0, "hello", 6);
  editor_insert_row(0, "world", 6);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 2, .y = 0, .render_x = DEFAULT_LNPAD};

  ok(editor.curs.x == 2, "sanity check");
  ok(editor.curs.y == 0, "sanity check");
  cursor_move_right();
  ok(editor.curs.x == 3, "moves right");
  ok(editor.curs.y == 0, "does not move the y coord");
  cursor_move_right();
  ok(editor.curs.x == 4, "moves right");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_right_at_end_of_line (void) {
  editor_insert_row(0, "hello", 6);
  editor_insert_row(0, "world", 6);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 6, .y = 0, .render_x = DEFAULT_LNPAD};

  ok(editor.curs.x == 6, "sanity check");
  ok(editor.curs.y == 0, "sanity check");
  cursor_move_right();
  ok(editor.curs.x == 0, "moves to start of next line");
  ok(editor.curs.y == 1, "next line");
  cursor_move_right();
  ok(editor.curs.x == 1, "moves right");
  ok(editor.curs.y == 1, "does not move the y coord");
}

void
test_cursor_move_right_at_end_of_last_line (void) {
  editor_insert_row(0, "hello", 6);
  editor_insert_row(0, "world", 6);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 6, .y = 1};

  ok(editor.curs.x == 6, "sanity check");
  ok(editor.curs.y == 1, "sanity check");
  cursor_move_right();
  ok(editor.curs.x == 6, "cannot move");
  ok(editor.curs.y == 1, "cannot move");
}

void
test_cursor_move_left_word (void) {
  editor_insert_row(0, "hello world what", 17);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 17, .y = 0, .render_x = DEFAULT_LNPAD};

  ok(editor.curs.x == 17, "sanity check");
  ok(editor.curs.y == 0, "sanity check");

  cursor_move_left_word();
  ok(editor.curs.x == 12, "jumps to the beginning of the preceding word");
  ok(editor.curs.y == 0, "does not move the y coord");

  cursor_move_left_word();
  ok(editor.curs.x == 11, "jumps to the end of the preceding word");
  ok(editor.curs.y == 0, "does not move the y coord");

  cursor_move_left_word();
  ok(editor.curs.x == 6, "jumps to the beginning of the preceding word");
  ok(editor.curs.y == 0, "does not move the y coord");

  cursor_move_left_word();
  ok(editor.curs.x == 5, "jumps to the end of the preceding word");
  ok(editor.curs.y == 0, "does not move the y coord");

  cursor_move_left_word();
  ok(editor.curs.x == 0, "jumps to the beginning");
  ok(editor.curs.y == 0, "does not move the y coord");

  cursor_move_left_word();
  ok(editor.curs.x == 0, "cannot move anymore");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_left_word_from_middle_of_word (void) {
  editor_insert_row(0, "hello world what", 17);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 16, .y = 0, .render_x = DEFAULT_LNPAD};

  ok(editor.curs.x == 16, "sanity check");
  ok(editor.curs.y == 0, "sanity check");

  cursor_move_left_word();
  ok(editor.curs.x == 12, "jumps to the beginning of the preceding word");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_left_word_no_breaks (void) {
  editor_insert_row(0, "helloworldwhat", 16);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 16, .y = 0, .render_x = DEFAULT_LNPAD};

  ok(editor.curs.x == 16, "sanity check");
  ok(editor.curs.y == 0, "sanity check");

  cursor_move_left_word();
  ok(editor.curs.x == 0, "jumps all the way to the beginning");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_left_word_all_breaks (void) {
  editor_insert_row(0, "                ", 16);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 16, .y = 0, .render_x = DEFAULT_LNPAD};

  ok(editor.curs.x == 16, "sanity check");
  ok(editor.curs.y == 0, "sanity check");
  fflush(stdout);
  cursor_move_left_word();

  ok(editor.curs.x == 0, "jumps all the way to the beginning");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_left_word_at_begin_of_line (void) {
  editor_insert_row(0, "hello world what", 17);
  editor_insert_row(0, "hello world what", 17);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 0, .y = 1};

  ok(editor.curs.x == 0, "sanity check");
  ok(editor.curs.y == 1, "sanity check");

  cursor_move_left_word();
  ok(editor.curs.x == 17, "jumps to the end to the previous line");
  ok(editor.curs.y == 0, "moves to the previous line");

  cursor_move_left_word();
  ok(editor.curs.x == 12, "jumps to the beginning of the preceding word");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_left_word_at_begin_of_first_line (void) {
  editor_insert_row(0, "hello world what", 17);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 0, .y = 0, .render_x = DEFAULT_LNPAD};

  ok(editor.curs.x == 0, "sanity check");
  ok(editor.curs.y == 0, "sanity check");

  cursor_move_left_word();
  ok(editor.curs.x == 0, "cannot move");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_left_word_prev_break_char (void) {
  editor_insert_row(0, "hello world   what", 19);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 14, .y = 0, .render_x = DEFAULT_LNPAD};

  ok(editor.curs.x == 14, "sanity check");
  ok(editor.curs.y == 0, "sanity check");

  cursor_move_left_word();
  ok(editor.curs.x == 11, "jumps to end of preceding word");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_left_word_prev_non_break_char (void) {
  editor_insert_row(0, "hello world   what", 19);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 11, .y = 0, .render_x = DEFAULT_LNPAD};

  ok(editor.curs.x == 11, "sanity check");
  ok(editor.curs.y == 0, "sanity check");

  cursor_move_left_word();
  ok(editor.curs.x == 6, "jumps to beginning of preceding word");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_right_word (void) {
  editor_insert_row(0, "hello world what", 17);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 0, .y = 0, .render_x = DEFAULT_LNPAD};

  ok(editor.curs.x == 0, "sanity check");
  ok(editor.curs.y == 0, "sanity check");

  cursor_move_right_word();
  ok(editor.curs.x == 5, "jumps to the end of the next word");
  ok(editor.curs.y == 0, "does not move the y coord");

  cursor_move_right_word();
  ok(editor.curs.x == 6, "jumps to the beginning of the next word");
  ok(editor.curs.y == 0, "does not move the y coord");

  cursor_move_right_word();
  ok(editor.curs.x == 11, "jumps to the end of the next word");
  ok(editor.curs.y == 0, "does not move the y coord");

  cursor_move_right_word();
  ok(editor.curs.x == 12, "jumps to the beginning of the next word");
  ok(editor.curs.y == 0, "does not move the y coord");

  cursor_move_right_word();
  ok(editor.curs.x == 17, "jumps to the end");
  ok(editor.curs.y == 0, "does not move the y coord");

  cursor_move_right_word();
  ok(editor.curs.x == 17, "cannot move anymore");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_right_word_from_middle_of_word (void) {
  editor_insert_row(0, "hello world what", 17);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 2, .y = 0, .render_x = DEFAULT_LNPAD};

  ok(editor.curs.x == 2, "sanity check");
  ok(editor.curs.y == 0, "sanity check");

  cursor_move_right_word();
  ok(editor.curs.x == 5, "jumps to the end of the next word");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_right_word_no_breaks (void) {
  editor_insert_row(0, "helloworldwhat", 16);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 0, .y = 0, .render_x = DEFAULT_LNPAD};

  ok(editor.curs.x == 0, "sanity check");
  ok(editor.curs.y == 0, "sanity check");

  cursor_move_right_word();
  ok(editor.curs.x == 16, "jumps all the way to the end");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_right_word_all_breaks (void) {
  editor_insert_row(0, "                ", 16);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 0, .y = 0, .render_x = DEFAULT_LNPAD};

  ok(editor.curs.x == 0, "sanity check");
  ok(editor.curs.y == 0, "sanity check");

  cursor_move_right_word();
  ok(editor.curs.x == 16, "jumps all the way to the end");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_right_word_at_end_of_line (void) {
  editor_insert_row(0, "hello", 6);
  editor_insert_row(0, "world", 6);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 6, .y = 0, .render_x = DEFAULT_LNPAD};

  ok(editor.curs.x == 6, "sanity check");
  ok(editor.curs.y == 0, "sanity check");
  cursor_move_right_word();
  ok(editor.curs.x == 0, "moves to start of next line");
  ok(editor.curs.y == 1, "next line");
  cursor_move_right_word();
  ok(editor.curs.x == 6, "moves to end of next word");
  ok(editor.curs.y == 1, "does not move the y coord");
}

void
test_cursor_move_right_word_at_end_of_last_line (void) {
  editor_insert_row(0, "hello world what", 17);
  editor_insert_row(0, "hello world what", 17);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 17, .y = 1};

  ok(editor.curs.x == 17, "sanity check");
  ok(editor.curs.y == 1, "sanity check");

  cursor_move_right_word();
  ok(editor.curs.x == 17, "cannot move");
  ok(editor.curs.y == 1, "does not move the y coord");
}

void
test_cursor_move_right_word_next_break_char (void) {
  editor_insert_row(0, "hello world   what", 19);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 11, .y = 0, .render_x = DEFAULT_LNPAD};

  ok(editor.curs.x == 11, "sanity check");
  ok(editor.curs.y == 0, "sanity check");

  cursor_move_right_word();
  ok(editor.curs.x == 14, "jumps to beginning of next word");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_right_word_next_non_break_char (void) {
  editor_insert_row(0, "hello world   what", 19);
  editor.curs = (cursor_t){.col_off = 0, .row_off = 0, .x = 14, .y = 0, .render_x = DEFAULT_LNPAD};

  ok(editor.curs.x == 14, "sanity check");
  ok(editor.curs.y == 0, "sanity check");

  cursor_move_right_word();
  ok(editor.curs.x == 19, "jumps to end of next word");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
run_cursor_tests (void) {
  void (*functions[])() = {
    test_cursor_move_down,
    test_cursor_move_down_at_bottom,
    test_cursor_move_down_one_line,

    test_cursor_move_up,
    test_cursor_move_up_at_top,
    test_cursor_move_up_one_line,

    test_cursor_move_left,
    test_cursor_move_left_at_begin_of_line,
    test_cursor_move_left_at_begin_of_first_line,

    test_cursor_move_right,
    test_cursor_move_right_at_end_of_line,
    test_cursor_move_right_at_end_of_last_line,

    test_cursor_move_left_word,
    test_cursor_move_left_word_from_middle_of_word,
    test_cursor_move_left_word_no_breaks,
    test_cursor_move_left_word_all_breaks,
    test_cursor_move_left_word_at_begin_of_line,
    test_cursor_move_left_word_at_begin_of_first_line,
    test_cursor_move_left_word_prev_break_char,
    test_cursor_move_left_word_prev_non_break_char,

    test_cursor_move_right_word,
    test_cursor_move_right_word_from_middle_of_word,
    test_cursor_move_right_word_no_breaks,
    test_cursor_move_right_word_all_breaks,
    test_cursor_move_right_word_at_end_of_line,
    test_cursor_move_right_word_at_end_of_last_line,
    test_cursor_move_right_word_next_break_char,
    test_cursor_move_right_word_next_non_break_char,
  };

  for (unsigned int i = 0; i < sizeof(functions) / sizeof(functions[0]); i++) {
    test_cursor_setup();
    functions[i]();
    test_cursor_teardown();
  }
}
