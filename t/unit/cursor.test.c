#include "cursor.h"

#include <stdlib.h>

#include "line_editor.h"
#include "tests.h"

size_t
tty_get_window_size (size_t *rows, size_t *cols) {
  *rows = 0;
  *cols = 0;
  return 0;
}

static void
setup (void) {
  editor_init(&editor);
}

static void
teardown (void) {
  editor_free(&editor);
}

static void
test_cursor_on_first_line (void) {
  line_editor_insert_newline(&editor.line_ed);
  line_editor_insert_newline(&editor.line_ed);
  line_editor_insert_newline(&editor.line_ed);

  SET_CURSOR(0, 0);
  ok(cursor_on_first_line(&editor.line_ed) == true, "cursor is on first line");

  SET_CURSOR(0, 1);
  ok(cursor_on_first_line(&editor.line_ed) == false, "cursor is not on first line");

  SET_CURSOR(0, 2);
  ok(cursor_on_first_line(&editor.line_ed) == false, "cursor is still not on first line");
}

static void
test_cursor_on_first_col (void) {
  line_editor_insert_newline(&editor.line_ed);
  line_editor_insert_newline(&editor.line_ed);
  line_editor_insert_newline(&editor.line_ed);

  SET_CURSOR(0, 0);
  ok(cursor_on_first_col(&editor.line_ed) == true, "cursor is on first col");

  SET_CURSOR(0, 1);
  ok(cursor_on_first_col(&editor.line_ed) == true, "cursor is still on first col");

  SET_CURSOR(0, 2);
  ok(cursor_on_first_col(&editor.line_ed) == true, "cursor is still on first col");

  SET_CURSOR(1, 0);
  ok(cursor_on_first_col(&editor.line_ed) == false, "cursor is not on first col");
}

static void
test_cursor_above_visible_window (void) {
  editor.line_ed.curs.row_off = 100;
  ok(cursor_above_visible_window(&editor.line_ed) == true, "above visible window when row_off is > y cursor coord");

  SET_CURSOR(0, 100);
  ok(cursor_above_visible_window(&editor.line_ed) == false, "not above visible window when row_off equals y cursor coord");

  SET_CURSOR(0, 101);
  ok(cursor_above_visible_window(&editor.line_ed) == false, "not above visible window when row_off less than y cursor coord");
}

static void
test_cursor_below_visible_window (void) {
  //   return editor.line_ed.curs.y >= editor.line_ed.curs.row_off + editor.win.rows;
  editor.win.rows             = 100;
  editor.line_ed.curs.row_off = 1;
  SET_CURSOR(0, 102);
  ok(cursor_below_visible_window(&editor.line_ed) == true, "below visible window when num rows + row_off is < y cursor coord");

  SET_CURSOR(0, 100);
  ok(cursor_below_visible_window(&editor.line_ed) == false, "below visible window when num rows + row_off equals y cursor coord");

  SET_CURSOR(0, 101);
  ok(
    cursor_below_visible_window(&editor.line_ed) == false,
    "not below visible window when num rows + row_off greater than y cursor "
    "coord"
  );
}

static void
test_cursor_left_of_visible_window (void) {
  editor.line_ed.curs.col_off = 100;
  SET_CURSOR(99, 0);
  ok(cursor_left_of_visible_window(&editor.line_ed) == true, "left of visible window when x cursor coord is less than col_off");

  SET_CURSOR(99, 5);
  ok(
    cursor_left_of_visible_window(&editor.line_ed) == true,
    "left of visible window when x cursor coord is less than col_off on any "
    "line"
  );

  SET_CURSOR(100, 0);
  ok(cursor_left_of_visible_window(&editor.line_ed) == false, "not left of visible window when x cursor coord is equal to col_off");

  SET_CURSOR(101, 0);
  ok(cursor_left_of_visible_window(&editor.line_ed) == false, "not left of visible window when x cursor coord is greater than col_off");
}

static void
test_cursor_right_of_visible_window (void) {
  editor.line_ed.curs.col_off = 10;
  editor.win.cols             = 10;
  line_pad                    = 3;

  SET_CURSOR(23, 0);
  ok(cursor_right_of_visible_window(&editor.line_ed) == true, "right of visible window");

  SET_CURSOR(16, 0);
  ok(cursor_right_of_visible_window(&editor.line_ed) == true, "right of visible window");

  SET_CURSOR(25, 1);
  ok(cursor_right_of_visible_window(&editor.line_ed) == true, "right of visible window");

  SET_CURSOR(15, 0);
  ok(cursor_right_of_visible_window(&editor.line_ed) == false, "not right of visible window");
}

static void
test_cursor_in_cell_zero (void) {
  SET_CURSOR(0, 0);
  ok(cursor_in_cell_zero(&editor.line_ed) == true, "cursor is in cell zero");

  SET_CURSOR(1, 0);
  ok(cursor_in_cell_zero(&editor.line_ed) == false, "cursor is not in cell zero");

  SET_CURSOR(0, 1);
  ok(cursor_in_cell_zero(&editor.line_ed) == false, "cursor is not in cell zero");
}

static void
test_cursor_not_at_row_begin (void) {
  SET_CURSOR(1, 0);
  ok(cursor_not_at_row_begin(&editor.line_ed) == true, "cursor is not at row begin");

  SET_CURSOR(0, 0);
  ok(cursor_not_at_row_begin(&editor.line_ed) == false, "cursor is at row begin");

  SET_CURSOR(0, 1);
  ok(cursor_not_at_row_begin(&editor.line_ed) == false, "cursor is at row begin");
}

static void
test_cursor_move_down (void) {
  line_editor_insert(&editor.line_ed, "hello\n");
  line_editor_insert(&editor.line_ed, "world\n");
  line_editor_insert(&editor.line_ed, "what\n");
  SET_CURSOR(1, 0);

  ok(editor.line_ed.curs.x == 1, "sanity check");
  ok(editor.line_ed.curs.y == 0, "sanity check");
  cursor_move_down(&editor.line_ed);
  ok(editor.line_ed.curs.x == 1, "does not modify x coord");
  ok(editor.line_ed.curs.y == 1, "increases y coord by 1");
  cursor_move_down(&editor.line_ed);
  ok(editor.line_ed.curs.x == 1, "does not modify x coord");
  ok(editor.line_ed.curs.y == 2, "increases y coord by 1");
}

static void
test_cursor_move_down_at_bottom (void) {
  line_editor_insert(&editor.line_ed, "hello\n");
  line_editor_insert(&editor.line_ed, "hello");
  SET_CURSOR(1, 1);

  ok(editor.line_ed.curs.x == 1, "sanity check");
  ok(editor.line_ed.curs.y == 1, "sanity check");
  cursor_move_down(&editor.line_ed);
  ok(editor.line_ed.curs.x == 1, "does not modify x coord");
  ok(editor.line_ed.curs.y == 1, "cannot move");
  cursor_move_down(&editor.line_ed);
  ok(editor.line_ed.curs.x == 1, "does not modify x coord");
  ok(editor.line_ed.curs.y == 1, "cannot move");
}

static void
test_cursor_move_down_one_line (void) {
  line_editor_insert(&editor.line_ed, "hello");
  SET_CURSOR(1, 0);

  ok(editor.line_ed.curs.x == 1, "sanity check");
  ok(editor.line_ed.curs.y == 0, "sanity check");
  cursor_move_down(&editor.line_ed);
  ok(editor.line_ed.curs.x == 1, "does not modify x coord");
  ok(editor.line_ed.curs.y == 0, "cannot move");
  cursor_move_down(&editor.line_ed);
  ok(editor.line_ed.curs.x == 1, "does not modify x coord");
  ok(editor.line_ed.curs.y == 0, "cannot move");
}

static void
test_cursor_move_up (void) {
  line_editor_insert(&editor.line_ed, "hello\n");
  line_editor_insert(&editor.line_ed, "world\n");
  line_editor_insert(&editor.line_ed, "what\n");
  SET_CURSOR(1, 2);

  ok(editor.line_ed.curs.x == 1, "sanity check");
  ok(editor.line_ed.curs.y == 2, "sanity check");
  cursor_move_up(&editor.line_ed);
  ok(editor.line_ed.curs.x == 1, "does not modify x coord");
  ok(editor.line_ed.curs.y == 1, "decreases y coord by 1");
  cursor_move_up(&editor.line_ed);
  ok(editor.line_ed.curs.x == 1, "does not modify x coord");
  ok(editor.line_ed.curs.y == 0, "decreases y coord by 1");
}

static void
test_cursor_move_up_at_top (void) {
  line_editor_insert(&editor.line_ed, "hello\n");
  line_editor_insert(&editor.line_ed, "world\n");
  line_editor_insert(&editor.line_ed, "what\n");
  SET_CURSOR(1, 0);

  ok(editor.line_ed.curs.x == 1, "sanity check");
  ok(editor.line_ed.curs.y == 0, "sanity check");
  cursor_move_up(&editor.line_ed);
  ok(editor.line_ed.curs.x == 1, "does not modify x coord");
  ok(editor.line_ed.curs.y == 0, "decreases y coord by 1");
  cursor_move_up(&editor.line_ed);
  ok(editor.line_ed.curs.x == 1, "does not modify x coord");
  ok(editor.line_ed.curs.y == 0, "decreases y coord by 1");
}

static void
test_cursor_move_up_one_line (void) {
  line_editor_insert(&editor.line_ed, "what\n");
  SET_CURSOR(1, 0);

  ok(editor.line_ed.curs.x == 1, "sanity check");
  ok(editor.line_ed.curs.y == 0, "sanity check");
  cursor_move_up(&editor.line_ed);
  ok(editor.line_ed.curs.x == 1, "does not modify x coord");
  ok(editor.line_ed.curs.y == 0, "decreases y coord by 1");
  cursor_move_up(&editor.line_ed);
  ok(editor.line_ed.curs.x == 1, "does not modify x coord");
  ok(editor.line_ed.curs.y == 0, "decreases y coord by 1");
}

static void
test_cursor_move_left (void) {
  line_editor_insert(&editor.line_ed, "hello\n");
  line_editor_insert(&editor.line_ed, "world\n");
  SET_CURSOR(2, 0);

  ok(editor.line_ed.curs.x == 2, "sanity check");
  ok(editor.line_ed.curs.y == 0, "sanity check");
  cursor_move_left(&editor.line_ed);
  ok(editor.line_ed.curs.x == 1, "moves left");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");
  cursor_move_left(&editor.line_ed);
  ok(editor.line_ed.curs.x == 0, "moves left");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");
}

static void
test_cursor_move_left_at_begin_of_line (void) {
  line_editor_insert(&editor.line_ed, "hello");
  SET_CURSOR(5, 0);

  line_editor_insert(&editor.line_ed, "\nworld");
  SET_CURSOR(0, 1);

  cursor_move_left(&editor.line_ed);
  ok(editor.line_ed.curs.x == 5, "moves to the end");
  ok(editor.line_ed.curs.y == 0, "moves up");

  cursor_move_left(&editor.line_ed);
  ok(editor.line_ed.curs.x == 4, "moves left");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");
}

static void
test_cursor_move_left_at_begin_of_first_line (void) {
  line_editor_insert(&editor.line_ed, "hello\n");
  line_editor_insert(&editor.line_ed, "world\n");
  SET_CURSOR(0, 0);

  ok(editor.line_ed.curs.x == 0, "sanity check");
  ok(editor.line_ed.curs.y == 0, "sanity check");
  cursor_move_left(&editor.line_ed);
  ok(editor.line_ed.curs.x == 0, "can't move left");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");
}

static void
test_cursor_move_right (void) {
  line_editor_insert(&editor.line_ed, "hello\n");
  line_editor_insert(&editor.line_ed, "world\n");
  SET_CURSOR(2, 0);

  ok(editor.line_ed.curs.x == 2, "sanity check");
  ok(editor.line_ed.curs.y == 0, "sanity check");
  cursor_move_right(&editor.line_ed);
  ok(editor.line_ed.curs.x == 3, "moves right");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");
  cursor_move_right(&editor.line_ed);
  ok(editor.line_ed.curs.x == 4, "moves right");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");
}

static void
test_cursor_move_right_at_end_of_line (void) {
  line_editor_insert(&editor.line_ed, "hello");
  SET_CURSOR(5, 0);
  line_editor_insert(&editor.line_ed, "\nworld");
  SET_CURSOR(5, 0);

  ok(editor.line_ed.curs.x == 5, "sanity check");
  ok(editor.line_ed.curs.y == 0, "sanity check");
  cursor_move_right(&editor.line_ed);
  ok(editor.line_ed.curs.x == 0, "moves to start of next line");
  ok(editor.line_ed.curs.y == 1, "next line");
  cursor_move_right(&editor.line_ed);
  ok(editor.line_ed.curs.x == 1, "moves right");
  ok(editor.line_ed.curs.y == 1, "does not move the y coord");
}

static void
test_cursor_move_right_at_end_of_last_line (void) {
  line_editor_insert(&editor.line_ed, "hello\n");
  line_editor_insert(&editor.line_ed, "world\n");
  SET_CURSOR(6, 1);

  ok(editor.line_ed.curs.x == 6, "sanity check");
  ok(editor.line_ed.curs.y == 1, "sanity check");
  cursor_move_right(&editor.line_ed);
  ok(editor.line_ed.curs.x == 6, "cannot move");
  ok(editor.line_ed.curs.y == 1, "cannot move");
}

static void
test_cursor_move_left_word (void) {
  line_editor_insert(&editor.line_ed, "hello world what\n");
  SET_CURSOR(17, 0);

  ok(editor.line_ed.curs.x == 17, "sanity check");
  ok(editor.line_ed.curs.y == 0, "sanity check");

  cursor_move_left_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 12, "jumps to the beginning of the preceding word");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");

  cursor_move_left_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 11, "jumps to the end of the preceding word");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");

  cursor_move_left_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 6, "jumps to the beginning of the preceding word");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");

  cursor_move_left_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 5, "jumps to the end of the preceding word");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");

  cursor_move_left_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 0, "jumps to the beginning");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");

  cursor_move_left_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 0, "cannot move anymore");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");
}

static void
test_cursor_move_left_word_from_middle_of_word (void) {
  line_editor_insert(&editor.line_ed, "hello world what\n");
  SET_CURSOR(16, 0);

  ok(editor.line_ed.curs.x == 16, "sanity check");
  ok(editor.line_ed.curs.y == 0, "sanity check");

  cursor_move_left_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 12, "jumps to the beginning of the preceding word");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");
}

static void
test_cursor_move_left_word_no_breaks (void) {
  line_editor_insert(&editor.line_ed, "helloworldwhat\n");
  SET_CURSOR(16, 0);

  ok(editor.line_ed.curs.x == 16, "sanity check");
  ok(editor.line_ed.curs.y == 0, "sanity check");

  cursor_move_left_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 0, "jumps all the way to the beginning");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");
}

static void
test_cursor_move_left_word_all_breaks (void) {
  line_editor_insert(&editor.line_ed, "                \n");
  SET_CURSOR(16, 0);

  ok(editor.line_ed.curs.x == 16, "sanity check");
  ok(editor.line_ed.curs.y == 0, "sanity check");
  fflush(stdout);
  cursor_move_left_word(&editor.line_ed);

  ok(editor.line_ed.curs.x == 0, "jumps all the way to the beginning");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");
}

static void
test_cursor_move_left_word_at_begin_of_line (void) {
  line_editor_insert(&editor.line_ed, "hello world what");
  SET_CURSOR(16, 0);
  line_editor_insert(&editor.line_ed, "\nhello world what");
  SET_CURSOR(0, 1);

  cursor_move_left_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 16, "jumps to the end to the previous line");
  ok(editor.line_ed.curs.y == 0, "moves to the previous line");

  cursor_move_left_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 12, "jumps to the beginning of the preceding word");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");
}

static void
test_cursor_move_left_word_at_begin_of_first_line (void) {
  line_editor_insert(&editor.line_ed, "hello world what\n");
  SET_CURSOR(0, 0);

  ok(editor.line_ed.curs.x == 0, "sanity check");
  ok(editor.line_ed.curs.y == 0, "sanity check");

  cursor_move_left_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 0, "cannot move");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");
}

static void
test_cursor_move_left_word_prev_break_char (void) {
  line_editor_insert(&editor.line_ed, "hello world   what\n");
  SET_CURSOR(14, 0);

  ok(editor.line_ed.curs.x == 14, "sanity check");
  ok(editor.line_ed.curs.y == 0, "sanity check");

  cursor_move_left_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 11, "jumps to end of preceding word");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");
}

static void
test_cursor_move_left_word_prev_non_break_char (void) {
  line_editor_insert(&editor.line_ed, "hello world   what\n");
  SET_CURSOR(11, 0);

  ok(editor.line_ed.curs.x == 11, "sanity check");
  ok(editor.line_ed.curs.y == 0, "sanity check");

  cursor_move_left_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 6, "jumps to beginning of preceding word");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");
}

static void
test_cursor_move_right_word (void) {
  line_editor_insert(&editor.line_ed, "hello world what");
  SET_CURSOR(0, 0);

  cursor_move_right_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 5, "jumps to the end of the next word");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");

  cursor_move_right_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 6, "jumps to the beginning of the next word");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");

  cursor_move_right_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 11, "jumps to the end of the next word");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");

  cursor_move_right_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 12, "jumps to the beginning of the next word");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");

  cursor_move_right_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 16, "jumps to the end");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");

  cursor_move_right_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 16, "cannot move anymore");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");
}

static void
test_cursor_move_right_word_from_middle_of_word (void) {
  line_editor_insert(&editor.line_ed, "hello world what\n");
  SET_CURSOR(2, 0);

  ok(editor.line_ed.curs.x == 2, "sanity check");
  ok(editor.line_ed.curs.y == 0, "sanity check");

  cursor_move_right_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 5, "jumps to the end of the next word");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");
}

static void
test_cursor_move_right_word_no_breaks (void) {
  line_editor_insert(&editor.line_ed, "helloworldwhat");
  SET_CURSOR(0, 0);

  ok(editor.line_ed.curs.x == 0, "sanity check");
  ok(editor.line_ed.curs.y == 0, "sanity check");

  cursor_move_right_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 14, "jumps all the way to the end");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");
}

static void
test_cursor_move_right_word_all_breaks (void) {
  line_editor_insert(&editor.line_ed, "                \n");
  SET_CURSOR(0, 0);

  ok(editor.line_ed.curs.x == 0, "sanity check");
  ok(editor.line_ed.curs.y == 0, "sanity check");

  cursor_move_right_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 16, "jumps all the way to the end");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");
}

static void
test_cursor_move_right_word_at_end_of_line (void) {
  line_editor_insert(&editor.line_ed, "hello");
  SET_CURSOR(5, 0);
  line_editor_insert(&editor.line_ed, "\nworld");

  cursor_move_right_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 0, "moves to start of next line");
  ok(editor.line_ed.curs.y == 1, "next line");

  cursor_move_right_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 5, "moves to end of next word");
  ok(editor.line_ed.curs.y == 1, "does not move the y coord");
}

static void
test_cursor_move_right_word_at_end_of_last_line (void) {
  line_editor_insert(&editor.line_ed, "hello world what\n");
  line_editor_insert(&editor.line_ed, "hello world what\n");
  SET_CURSOR(17, 1);

  ok(editor.line_ed.curs.x == 17, "sanity check");
  ok(editor.line_ed.curs.y == 1, "sanity check");

  cursor_move_right_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 17, "cannot move");
  ok(editor.line_ed.curs.y == 1, "does not move the y coord");
}

static void
test_cursor_move_right_word_next_break_char (void) {
  line_editor_insert(&editor.line_ed, "hello world   what\n");
  SET_CURSOR(11, 0);

  ok(editor.line_ed.curs.x == 11, "sanity check");
  ok(editor.line_ed.curs.y == 0, "sanity check");

  cursor_move_right_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 14, "jumps to beginning of next word");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");
}

static void
test_cursor_move_right_word_next_non_break_char (void) {
  line_editor_insert(&editor.line_ed, "hello world   what");
  SET_CURSOR(14, 0);

  cursor_move_right_word(&editor.line_ed);
  ok(editor.line_ed.curs.x == 18, "jumps to end of next word");
  ok(editor.line_ed.curs.y == 0, "does not move the y coord");
}

static void
test_cursor_move_top (void) {
  SET_CURSOR(2, 1000);
  cursor_move_top(&editor.line_ed);
  ok(editor.line_ed.curs.x == 2, "maintains x");
  ok(editor.line_ed.curs.y == 0, "moves y to zero");

  SET_CURSOR(2, 0);
  cursor_move_top(&editor.line_ed);
  ok(editor.line_ed.curs.x == 2, "maintains x");
  ok(editor.line_ed.curs.y == 0, "leaves y at zero");
}

static void
test_cursor_move_visible_top (void) {
  editor.line_ed.curs.row_off = 12;

  SET_CURSOR(2, 1000);
  cursor_move_visible_top(&editor.line_ed);
  ok(editor.line_ed.curs.x == 2, "maintains x");
  ok(editor.line_ed.curs.y == 12, "moves y to row_off");

  SET_CURSOR(2, 12);
  cursor_move_visible_top(&editor.line_ed);
  ok(editor.line_ed.curs.x == 2, "maintains x");
  ok(editor.line_ed.curs.y == 12, "leaves y at row_off");
}

static void
test_cursor_move_bottom (void) {
  editor.win.rows = 100;

  SET_CURSOR(2, 1000);
  cursor_move_bottom(&editor.line_ed);
  ok(editor.line_ed.curs.x == 2, "maintains x");
  ok(editor.line_ed.curs.y == 99, "moves y to numrows minus 1");

  SET_CURSOR(2, 0);
  cursor_move_bottom(&editor.line_ed);
  ok(editor.line_ed.curs.x == 2, "maintains x");
  ok(editor.line_ed.curs.y == 99, "leaves y at numrows minus 1");
}

static void
test_cursor_move_visible_bottom (void) {
  editor.line_ed.r->num_lines = 100;
  editor.line_ed.curs.row_off = 5;
  editor.win.rows             = 100;

  SET_CURSOR(15, 101);
  cursor_move_visible_bottom(&editor.line_ed);
  ok(editor.line_ed.curs.x == 15, "maintains x");
  ok(editor.line_ed.curs.y == 100, "sets y to num_lines");

  SET_CURSOR(15, 100);
  cursor_move_visible_bottom(&editor.line_ed);
  ok(editor.line_ed.curs.x == 15, "maintains x");
  ok(editor.line_ed.curs.y == 104, "sets y to row_off plus numrows minus 1");

  SET_CURSOR(15, 0);
  cursor_move_visible_bottom(&editor.line_ed);
  ok(editor.line_ed.curs.x == 15, "maintains x");
  ok(editor.line_ed.curs.y == 104, "sets y to row_off plus numrows minus 1");
}

static void
test_cursor_move_begin (void) {
  SET_CURSOR(15, 0);
  cursor_move_begin(&editor.line_ed);
  ok(editor.line_ed.curs.x == 0, "sets x to zero");
  ok(editor.line_ed.curs.y == 0, "maintains y");

  SET_CURSOR(0, 0);
  cursor_move_begin(&editor.line_ed);
  ok(editor.line_ed.curs.x == 0, "keeps x at zero");
  ok(editor.line_ed.curs.y == 0, "maintains y");

  SET_CURSOR(15, 5);
  cursor_move_begin(&editor.line_ed);
  ok(editor.line_ed.curs.x == 0, "sets x to zero");
  ok(editor.line_ed.curs.y == 5, "maintains y");
}

static void
test_cursor_move_end (void) {
  line_editor_insert(&editor.line_ed, "endorsed by cheneys is not a good look");

  SET_CURSOR(15, 0);
  cursor_move_end(&editor.line_ed);
  ok(editor.line_ed.curs.x == 38, "sets x to line length (end of row)");
  ok(editor.line_ed.curs.y == 0, "maintains y");

  SET_CURSOR(0, 0);
  cursor_move_end(&editor.line_ed);
  ok(editor.line_ed.curs.x == 38, "sets x to line length (end of row)");
  ok(editor.line_ed.curs.y == 0, "maintains y");

  SET_CURSOR(38, 5);
  cursor_move_end(&editor.line_ed);
  ok(editor.line_ed.curs.x == 38, "maintains x at line length (end of row)");
  ok(editor.line_ed.curs.y == 5, "maintains y");

  SET_CURSOR(0, 500);
  cursor_move_end(&editor.line_ed);
  ok(editor.line_ed.curs.x == 0, "no-ops if the y coord is greater than the num of lines");
  ok(editor.line_ed.curs.y == 500, "no-ops");
}

static void
test_cursor_snap_to_end (void) {
  SET_CURSOR(0, 0);
  line_editor_insert(&editor.line_ed, "endorsed by cheneys is not a good look");
  SET_CURSOR(38, 0);
  line_editor_insert_newline(&editor.line_ed);
  SET_CURSOR(0, 1);
  line_editor_insert(&editor.line_ed, "lol");

  SET_CURSOR(3, 1);
  cursor_move_up(&editor.line_ed);
  cursor_snap_to_end(&editor.line_ed);
  ok(editor.line_ed.curs.x == 3, "maintains x when moving into a longer line");
  ok(editor.line_ed.curs.y == 0, "decrements y");

  cursor_move_end(&editor.line_ed);
  ok(editor.line_ed.curs.x == 38, "sanity check");
  ok(editor.line_ed.curs.y == 0, "sanity check");
  cursor_move_down(&editor.line_ed);
  cursor_snap_to_end(&editor.line_ed);
  ok(editor.line_ed.curs.x == 3, "moves x to the end of the now-current line");
  ok(editor.line_ed.curs.y == 1, "increments y");
}

static void
test_cursor_select_left (void) {
  line_editor_insert(&editor.line_ed, "hello world\ngoodbye world\nhello world\ngoodbye world\nhello world");
  SET_CURSOR(3, 4);
  ok(editor.line_ed.curs.select_active == false, "sanity check");

  cursor_select_left(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "selects one cell to the left");
  ok(editor.line_ed.curs.select_anchor.x == 3, "selects one cell to the left");
  ok(editor.line_ed.curs.select_anchor.y == 4, "selects one cell to the left");
  ok(editor.line_ed.curs.select_offset.x == 2, "selects one cell to the left");
  ok(editor.line_ed.curs.select_offset.y == 4, "selects one cell to the left");
  ok(editor.line_ed.curs.x == 2, "selects one cell to the left");
  ok(editor.line_ed.curs.y == 4, "selects one cell to the left");

  cursor_select_left(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "selects one cell to the left");
  ok(editor.line_ed.curs.select_anchor.x == 3, "selects one cell to the left");
  ok(editor.line_ed.curs.select_anchor.y == 4, "selects one cell to the left");
  ok(editor.line_ed.curs.select_offset.x == 1, "selects one cell to the left");
  ok(editor.line_ed.curs.select_offset.y == 4, "selects one cell to the left");
  ok(editor.line_ed.curs.x == 1, "selects one cell to the left");
  ok(editor.line_ed.curs.y == 4, "selects one cell to the left");

  cursor_select_left(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "selects one cell to the left");
  ok(editor.line_ed.curs.select_anchor.x == 3, "selects one cell to the left");
  ok(editor.line_ed.curs.select_anchor.y == 4, "selects one cell to the left");
  ok(editor.line_ed.curs.select_offset.x == 0, "selects one cell to the left");
  ok(editor.line_ed.curs.select_offset.y == 4, "selects one cell to the left");
  ok(editor.line_ed.curs.x == 0, "selects one cell to the left");
  ok(editor.line_ed.curs.y == 4, "selects one cell to the left");

  cursor_select_left(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "selects one cell to the left");
  ok(editor.line_ed.curs.select_anchor.x == 3, "selects one cell to the left");
  ok(editor.line_ed.curs.select_anchor.y == 4, "selects one cell to the left");
  ok(editor.line_ed.curs.select_offset.x == 13, "wraps x around");
  ok(editor.line_ed.curs.select_offset.y == 3, "moves to the previous line");
  ok(editor.line_ed.curs.x == 13, "selects one cell to the left");
  ok(editor.line_ed.curs.y == 3, "selects one cell to the left");
  ok(cursor_is_select_ltr(&editor.line_ed) == false, "is rtl");

  editor.line_ed.curs.select_active = false;

  cursor_select_left(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "selects one cell to the left");
  ok(editor.line_ed.curs.select_anchor.x == 13, "resets the x anchor when going active for the first time");
  ok(editor.line_ed.curs.select_anchor.y == 3, "resets the y anchor when going active for the first time");
  ok(editor.line_ed.curs.select_offset.x == 12, "selects one cell to the left");
  ok(editor.line_ed.curs.select_offset.y == 3, "selects one cell to the left");
  ok(editor.line_ed.curs.x == 12, "selects one cell to the left");
  ok(editor.line_ed.curs.y == 3, "selects one cell to the left");
}

static void
test_cursor_select_right (void) {
  line_editor_insert(&editor.line_ed, "hello world\ngoodbye world\nhello world\ngoodbye world\nhello world");
  SET_CURSOR(5, 0);
  ok(editor.line_ed.curs.select_active == false, "sanity check");

  cursor_select_right(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "selects one cell to the right");
  ok(editor.line_ed.curs.select_anchor.x == 5, "selects one cell to the right");
  ok(editor.line_ed.curs.select_anchor.y == 0, "selects one cell to the right");
  ok(editor.line_ed.curs.select_offset.x == 6, "selects one cell to the right");
  ok(editor.line_ed.curs.select_offset.y == 0, "selects one cell to the right");
  ok(editor.line_ed.curs.x == 6, "selects one cell to the right");
  ok(editor.line_ed.curs.y == 0, "selects one cell to the right");

  CALL_N_TIMES(3, cursor_select_right(&editor.line_ed));
  ok(editor.line_ed.curs.select_active == true, "selects 3 cells to the right");
  ok(editor.line_ed.curs.select_anchor.x == 5, "selects 3 cells to the right");
  ok(editor.line_ed.curs.select_anchor.y == 0, "selects 3 cells to the right");
  ok(editor.line_ed.curs.select_offset.x == 9, "selects 3 cells to the right");
  ok(editor.line_ed.curs.select_offset.y == 0, "selects 3 cells to the right");
  ok(editor.line_ed.curs.x == 9, "selects 3 cells to the right");
  ok(editor.line_ed.curs.y == 0, "selects 3 cells to the right");

  CALL_N_TIMES(4, cursor_select_right(&editor.line_ed));
  ok(editor.line_ed.curs.select_active == true, "selects 4 cells to the right");
  ok(editor.line_ed.curs.select_anchor.x == 5, "selects 4 cells to the right");
  ok(editor.line_ed.curs.select_anchor.y == 0, "selects 4 cells to the right");
  ok(editor.line_ed.curs.select_offset.x == 1, "wraps around to the next line");
  ok(editor.line_ed.curs.select_offset.y == 1, "wraps around to the next line");
  ok(editor.line_ed.curs.x == 1, "wraps around to the next line");
  ok(editor.line_ed.curs.y == 1, "wraps around to the next line");

  CALL_N_TIMES(13, cursor_select_right(&editor.line_ed));
  ok(editor.line_ed.curs.select_active == true, "selects to the right");
  ok(editor.line_ed.curs.select_anchor.x == 5, "selects to the right");
  ok(editor.line_ed.curs.select_anchor.y == 0, "selects to the right");
  ok(editor.line_ed.curs.select_offset.x == 0, "selects to the right");
  ok(editor.line_ed.curs.select_offset.y == 2, "wraps around to the next line");
  ok(editor.line_ed.curs.x == 0, "selects to the right");
  ok(editor.line_ed.curs.y == 2, "wraps around to the next line");

  cursor_select_clear(&editor.line_ed);
  cursor_select_right(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "selects to the right");
  ok(editor.line_ed.curs.select_anchor.x == 0, "selects to the right");
  ok(editor.line_ed.curs.select_anchor.y == 2, "selects to the right");
  ok(editor.line_ed.curs.select_offset.x == 1, "selects to the right");
  ok(editor.line_ed.curs.select_offset.y == 2, "selects to the right");
  ok(editor.line_ed.curs.x == 1, "selects to the right");
  ok(editor.line_ed.curs.y == 2, "selects to the right");
  ok(cursor_is_select_ltr(&editor.line_ed) == true, "is ltr");
}

static void
test_cursor_select_up (void) {
  line_editor_insert(&editor.line_ed, "hello world\ngoodbye world\nhello world\ngoodbye world\nhello world");
  SET_CURSOR(3, 1);
  ok(editor.line_ed.curs.select_active == false, "sanity check");

  cursor_select_up(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "select up selects all text from the anchor to the offset");
  ok(editor.line_ed.curs.select_anchor.x == 3, "select up selects all text from the anchor to the offset");
  ok(editor.line_ed.curs.select_anchor.y == 1, "select up selects all text from the anchor to the offset");
  ok(editor.line_ed.curs.select_offset.x == 3, "select up selects all text from the anchor to the offset");
  ok(editor.line_ed.curs.select_offset.y == 0, "select up selects all text from the anchor to the offset");
  ok(editor.line_ed.curs.x == 3, "select up selects all text from the anchor to the offset");
  ok(editor.line_ed.curs.y == 0, "select up selects all text from the anchor to the offset");
  ok(cursor_is_select_ltr(&editor.line_ed) == false, "select up selects all text from the anchor to the offset");

  // Micro-feature: pressing select-up while on the first line should select the
  // entire line to the beginning thereof
  cursor_select_up(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "select up 2x on the first line selects to the beginning of the line");
  ok(editor.line_ed.curs.select_anchor.x == 3, "select up 2x on the first line selects to the beginning of the line");
  ok(editor.line_ed.curs.select_anchor.y == 1, "select up 2x on the first line selects to the beginning of the line");
  ok(editor.line_ed.curs.select_offset.x == 0, "select up 2x on the first line selects to the beginning of the line");
  ok(editor.line_ed.curs.select_offset.y == 0, "select up 2x on the first line selects to the beginning of the line");
  ok(editor.line_ed.curs.x == 0, "select up 2x on the first line selects to the beginning of the line");
  ok(editor.line_ed.curs.y == 0, "select up 2x on the first line selects to the beginning of the line");
  ok(cursor_is_select_ltr(&editor.line_ed) == false, "select up 2x on the first line selects to the beginning of the line");

  cursor_select_clear(&editor.line_ed);
  SET_CURSOR(3, 1);
  CALL_N_TIMES(3, cursor_select_right(&editor.line_ed));
  cursor_select_up(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "select up maintains anchor but resets the offset");
  ok(editor.line_ed.curs.select_anchor.x == 3, "select up maintains anchor but resets the offset");
  ok(editor.line_ed.curs.select_anchor.y == 1, "select up maintains anchor but resets the offset");
  ok(editor.line_ed.curs.select_offset.x == 6, "select up maintains anchor but resets the offset");
  ok(editor.line_ed.curs.select_offset.y == 0, "select up maintains anchor but resets the offset");
  ok(editor.line_ed.curs.x == 6, "select up maintains anchor but resets the offset");
  ok(editor.line_ed.curs.y == 0, "select up maintains anchor but resets the offset");
  ok(cursor_is_select_ltr(&editor.line_ed) == false, "select up maintains anchor but resets the offset");
}

static void
test_cursor_select_down (void) {
  line_editor_insert(&editor.line_ed, "hello world\ngoodbye world\nhello world\ngoodbye world\nhello world");
  SET_CURSOR(3, 3);
  ok(editor.line_ed.curs.select_active == false, "sanity check");

  cursor_select_down(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "select down selects all text from the anchor to the offset");
  ok(editor.line_ed.curs.select_anchor.x == 3, "select down selects all text from the anchor to the offset");
  ok(editor.line_ed.curs.select_anchor.y == 3, "select down selects all text from the anchor to the offset");
  ok(editor.line_ed.curs.select_offset.x == 3, "select down selects all text from the anchor to the offset");
  ok(editor.line_ed.curs.select_offset.y == 4, "select down selects all text from the anchor to the offset");
  ok(editor.line_ed.curs.x == 3, "select down selects all text from the anchor to the offset");
  ok(editor.line_ed.curs.y == 4, "select down selects all text from the anchor to the offset");
  ok(cursor_is_select_ltr(&editor.line_ed) == true, "select down selects all text from the anchor to the offset");

  cursor_select_down(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "select down selects all text from the anchor to the end of the line");
  ok(editor.line_ed.curs.select_anchor.x == 3, "select down selects all text from the anchor to the end of the line");
  ok(editor.line_ed.curs.select_anchor.y == 3, "select down selects all text from the anchor to the end of the line");
  ok(editor.line_ed.curs.select_offset.x == 11, "select down selects all text from the anchor to the end of the line");
  ok(editor.line_ed.curs.select_offset.y == 4, "select down selects all text from the anchor to the end of the line");
  ok(editor.line_ed.curs.x == 11, "select down selects all text from the anchor to the end of the line");
  ok(editor.line_ed.curs.y == 4, "select down selects all text from the anchor to the end of the line");
  ok(cursor_is_select_ltr(&editor.line_ed) == true, "select down selects all text from the anchor to the end of the line");

  cursor_select_down(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "no-ops");
  ok(editor.line_ed.curs.select_anchor.x == 3, "no-ops");
  ok(editor.line_ed.curs.select_anchor.y == 3, "no-ops");
  ok(editor.line_ed.curs.select_offset.x == 11, "no-ops");
  ok(editor.line_ed.curs.select_offset.y == 4, "no-ops");
  ok(editor.line_ed.curs.x == 11, "no-ops");
  ok(editor.line_ed.curs.y == 4, "no-ops");
  ok(cursor_is_select_ltr(&editor.line_ed) == true, "no-ops");
}

static void
test_cursor_select_right_word (void) {
  line_editor_insert(&editor.line_ed, "hello world\ngoodbye world\nhello world\ngoodbye world\nhello world");
  SET_CURSOR(0, 0);
  ok(editor.line_ed.curs.select_active == false, "sanity check");

  cursor_select_right_word(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "selects to the right by one word");
  ok(editor.line_ed.curs.select_anchor.x == 0, "selects to the right by one word");
  ok(editor.line_ed.curs.select_anchor.y == 0, "selects to the right by one word");
  ok(editor.line_ed.curs.select_offset.x == 5, "selects to the right by one word");
  ok(editor.line_ed.curs.select_offset.y == 0, "selects to the right by one word");
  ok(editor.line_ed.curs.x == 5, "selects to the right by one word");
  ok(editor.line_ed.curs.y == 0, "selects to the right by one word");

  cursor_select_right_word(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "selects to the right by one word");
  ok(editor.line_ed.curs.select_anchor.x == 0, "selects to the right by one word");
  ok(editor.line_ed.curs.select_anchor.y == 0, "selects to the right by one word");
  ok(editor.line_ed.curs.select_offset.x == 6, "selects to the right by one word");
  ok(editor.line_ed.curs.select_offset.y == 0, "selects to the right by one word");
  ok(editor.line_ed.curs.x == 6, "selects to the right by one word");
  ok(editor.line_ed.curs.y == 0, "selects to the right by one word");

  cursor_select_right_word(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "selects to the right by one word");
  ok(editor.line_ed.curs.select_anchor.x == 0, "selects to the right by one word");
  ok(editor.line_ed.curs.select_anchor.y == 0, "selects to the right by one word");
  ok(editor.line_ed.curs.select_offset.x == 11, "selects to the right by one word");
  ok(editor.line_ed.curs.select_offset.y == 0, "selects to the right by one word");
  ok(editor.line_ed.curs.x == 11, "selects to the right by one word");
  ok(editor.line_ed.curs.y == 0, "selects to the right by one word");

  cursor_select_right_word(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "selects to the right by one word");
  ok(editor.line_ed.curs.select_anchor.x == 0, "selects to the right by one word");
  ok(editor.line_ed.curs.select_anchor.y == 0, "selects to the right by one word");
  ok(editor.line_ed.curs.select_offset.x == 0, "selects to the right by one word");
  ok(editor.line_ed.curs.select_offset.y == 1, "selects to the right by one word");
  ok(editor.line_ed.curs.x == 0, "selects to the right by one word");
  ok(editor.line_ed.curs.y == 1, "selects to the right by one word");

  cursor_select_clear(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == false, "selects to the right by one word");
  cursor_select_right_word(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "selects to the right by one word");
  ok(editor.line_ed.curs.select_anchor.x == 0, "selects to the right by one word");
  ok(editor.line_ed.curs.select_anchor.y == 1, "selects to the right by one word");
  ok(editor.line_ed.curs.select_offset.x == 7, "selects to the right by one word");
  ok(editor.line_ed.curs.select_offset.y == 1, "selects to the right by one word");
  ok(editor.line_ed.curs.x == 7, "selects to the right by one word");
  ok(editor.line_ed.curs.y == 1, "selects to the right by one word");
}

static void
test_cursor_select_left_word (void) {
  line_editor_insert(&editor.line_ed, "hello world\ngoodbye world\nhello world\ngoodbye world\nhello world");
  SET_CURSOR(7, 4);
  ok(editor.line_ed.curs.select_active == false, "sanity check");

  cursor_select_left_word(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "selects to the left by one word");
  ok(editor.line_ed.curs.select_anchor.x == 7, "selects to the left by one word");
  ok(editor.line_ed.curs.select_anchor.y == 4, "selects to the left by one word");
  ok(editor.line_ed.curs.select_offset.x == 6, "selects to the left by one word");
  ok(editor.line_ed.curs.select_offset.y == 4, "selects to the left by one word");
  ok(editor.line_ed.curs.x == 6, "selects to the left by one word");
  ok(editor.line_ed.curs.y == 4, "selects to the left by one word");

  cursor_select_left_word(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "selects to the left by one word");
  ok(editor.line_ed.curs.select_anchor.x == 7, "selects to the left by one word");
  ok(editor.line_ed.curs.select_anchor.y == 4, "selects to the left by one word");
  ok(editor.line_ed.curs.select_offset.x == 5, "selects to the left by one word");
  ok(editor.line_ed.curs.select_offset.y == 4, "selects to the left by one word");
  ok(editor.line_ed.curs.x == 5, "selects to the left by one word");
  ok(editor.line_ed.curs.y == 4, "selects to the left by one word");

  cursor_select_left_word(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "selects to the left by one word");
  ok(editor.line_ed.curs.select_anchor.x == 7, "selects to the left by one word");
  ok(editor.line_ed.curs.select_anchor.y == 4, "selects to the left by one word");
  ok(editor.line_ed.curs.select_offset.x == 0, "selects to the left by one word");
  ok(editor.line_ed.curs.select_offset.y == 4, "selects to the left by one word");
  ok(editor.line_ed.curs.x == 0, "selects to the left by one word");
  ok(editor.line_ed.curs.y == 4, "selects to the left by one word");

  cursor_select_left_word(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "selects to the left by one word");
  ok(editor.line_ed.curs.select_anchor.x == 7, "selects to the left by one word");
  ok(editor.line_ed.curs.select_anchor.y == 4, "selects to the left by one word");
  ok(editor.line_ed.curs.select_offset.x == 13, "selects to the left by one word");
  ok(editor.line_ed.curs.select_offset.y == 3, "selects to the left by one word");
  ok(editor.line_ed.curs.x == 13, "selects to the left by one word");
  ok(editor.line_ed.curs.y == 3, "selects to the left by one word");

  cursor_select_left_word(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "selects to the left by one word");
  ok(editor.line_ed.curs.select_anchor.x == 7, "selects to the left by one word");
  ok(editor.line_ed.curs.select_anchor.y == 4, "selects to the left by one word");
  ok(editor.line_ed.curs.select_offset.x == 8, "selects to the left by one word");
  ok(editor.line_ed.curs.select_offset.y == 3, "selects to the left by one word");
  ok(editor.line_ed.curs.x == 8, "selects to the left by one word");
  ok(editor.line_ed.curs.y == 3, "selects to the left by one word");

  cursor_select_left_word(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "selects to the left by one word");
  ok(editor.line_ed.curs.select_anchor.x == 7, "selects to the left by one word");
  ok(editor.line_ed.curs.select_anchor.y == 4, "selects to the left by one word");
  ok(editor.line_ed.curs.select_offset.x == 7, "selects to the left by one word");
  ok(editor.line_ed.curs.select_offset.y == 3, "selects to the left by one word");
  ok(editor.line_ed.curs.x == 7, "selects to the left by one word");
  ok(editor.line_ed.curs.y == 3, "selects to the left by one word");

  cursor_select_left_word(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "selects to the left by one word");
  ok(editor.line_ed.curs.select_anchor.x == 7, "selects to the left by one word");
  ok(editor.line_ed.curs.select_anchor.y == 4, "selects to the left by one word");
  ok(editor.line_ed.curs.select_offset.x == 0, "selects to the left by one word");
  ok(editor.line_ed.curs.select_offset.y == 3, "selects to the left by one word");
  ok(editor.line_ed.curs.x == 0, "selects to the left by one word");
  ok(editor.line_ed.curs.y == 3, "selects to the left by one word");
}

static void
test_cursor_select_up_word (void) {
  todo_start("need to implement: multi-cursor feature");
  todo_end();
}

static void
test_cursor_select_down_word (void) {
  todo_start("need to implement: multi-cursor feature");
  todo_end();
}

static void
test_cursor_select_x_axis (void) {
  line_editor_insert(&editor.line_ed, "hello world\ngoodbye world\nhello world\ngoodbye world\nhello world");
  SET_CURSOR(1, 1);
  ok(editor.line_ed.curs.select_active == false, "sanity check");

  cursor_select_right(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "selects to the right");
  ok(editor.line_ed.curs.select_anchor.x == 1, "selects to the right");
  ok(editor.line_ed.curs.select_anchor.y == 1, "selects to the right");
  ok(editor.line_ed.curs.select_offset.x == 2, "selects to the right");
  ok(editor.line_ed.curs.select_offset.y == 1, "selects to the right");
  ok(editor.line_ed.curs.x == 2, "selects to the right");
  ok(editor.line_ed.curs.y == 1, "selects to the right");
  ok(cursor_is_select_ltr(&editor.line_ed) == true, "is ltr");

  cursor_select_right_word(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "selects to the right");
  ok(editor.line_ed.curs.select_anchor.x == 1, "selects to the right");
  ok(editor.line_ed.curs.select_anchor.y == 1, "selects to the right");
  ok(editor.line_ed.curs.select_offset.x == 7, "selects to the right");
  ok(editor.line_ed.curs.select_offset.y == 1, "selects to the right");
  ok(editor.line_ed.curs.x == 7, "selects to the right");
  ok(editor.line_ed.curs.y == 1, "selects to the right");
  ok(cursor_is_select_ltr(&editor.line_ed) == true, "is ltr");

  CALL_N_TIMES(2, cursor_select_right(&editor.line_ed));
  ok(editor.line_ed.curs.select_active == true, "selects to the right");
  ok(editor.line_ed.curs.select_anchor.x == 1, "selects to the right");
  ok(editor.line_ed.curs.select_anchor.y == 1, "selects to the right");
  ok(editor.line_ed.curs.select_offset.x == 9, "selects to the right");
  ok(editor.line_ed.curs.select_offset.y == 1, "selects to the right");
  ok(editor.line_ed.curs.x == 9, "selects to the right");
  ok(editor.line_ed.curs.y == 1, "selects to the right");
  ok(cursor_is_select_ltr(&editor.line_ed) == true, "is ltr");

  CALL_N_TIMES(3, cursor_select_right_word(&editor.line_ed));
  ok(editor.line_ed.curs.select_active == true, "selects to the right");
  ok(editor.line_ed.curs.select_anchor.x == 1, "selects to the right");
  ok(editor.line_ed.curs.select_anchor.y == 1, "selects to the right");
  ok(editor.line_ed.curs.select_offset.x == 5, "selects to the right");
  ok(editor.line_ed.curs.select_offset.y == 2, "selects to the right");
  ok(editor.line_ed.curs.x == 5, "selects to the right");
  ok(editor.line_ed.curs.y == 2, "selects to the right");
  ok(cursor_is_select_ltr(&editor.line_ed) == true, "is ltr");

  cursor_select_left(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "selects to the left");
  ok(editor.line_ed.curs.select_anchor.x == 1, "selects to the left");
  ok(editor.line_ed.curs.select_anchor.y == 1, "selects to the left");
  ok(editor.line_ed.curs.select_offset.x == 4, "selects to the left");
  ok(editor.line_ed.curs.select_offset.y == 2, "selects to the left");
  ok(editor.line_ed.curs.x == 4, "selects to the left");
  ok(editor.line_ed.curs.y == 2, "selects to the left");
  ok(cursor_is_select_ltr(&editor.line_ed) == true, "is still ltr doe");

  cursor_select_left_word(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "selects to the left");
  ok(editor.line_ed.curs.select_anchor.x == 1, "selects to the left");
  ok(editor.line_ed.curs.select_anchor.y == 1, "selects to the left");
  ok(editor.line_ed.curs.select_offset.x == 0, "selects to the left");
  ok(editor.line_ed.curs.select_offset.y == 2, "selects to the left");
  ok(editor.line_ed.curs.x == 0, "selects to the left");
  ok(editor.line_ed.curs.y == 2, "selects to the left");
  ok(cursor_is_select_ltr(&editor.line_ed) == true, "is still ltr doe");

  CALL_N_TIMES(3, cursor_select_left(&editor.line_ed));
  ok(editor.line_ed.curs.select_active == true, "selects to the left");
  ok(editor.line_ed.curs.select_anchor.x == 1, "selects to the left");
  ok(editor.line_ed.curs.select_anchor.y == 1, "selects to the left");
  ok(editor.line_ed.curs.select_offset.x == 11, "selects to the left");
  ok(editor.line_ed.curs.select_offset.y == 1, "selects to the left");
  ok(editor.line_ed.curs.x == 11, "selects to the left");
  ok(editor.line_ed.curs.y == 1, "selects to the left");
  ok(cursor_is_select_ltr(&editor.line_ed) == true, "is ltr");

  CALL_N_TIMES(4, cursor_select_left_word(&editor.line_ed));
  ok(editor.line_ed.curs.select_active == true, "selects to the left");
  ok(editor.line_ed.curs.select_anchor.x == 1, "selects to the left");
  ok(editor.line_ed.curs.select_anchor.y == 1, "selects to the left");
  ok(editor.line_ed.curs.select_offset.x == 11, "selects to the left");
  ok(editor.line_ed.curs.select_offset.y == 0, "selects to the left");
  ok(editor.line_ed.curs.x == 11, "selects to the left");
  ok(editor.line_ed.curs.y == 0, "selects to the left");
  ok(cursor_is_select_ltr(&editor.line_ed) == false, "now is rtl");

  cursor_select_left_word(&editor.line_ed);
  ok(editor.line_ed.curs.select_active == true, "selects to the left");
  ok(editor.line_ed.curs.select_anchor.x == 1, "selects to the left");
  ok(editor.line_ed.curs.select_anchor.y == 1, "selects to the left");
  ok(editor.line_ed.curs.select_offset.x == 6, "selects to the left");
  ok(editor.line_ed.curs.select_offset.y == 0, "selects to the left");
  ok(editor.line_ed.curs.x == 6, "selects to the left");
  ok(editor.line_ed.curs.y == 0, "selects to the left");
  ok(cursor_is_select_ltr(&editor.line_ed) == false, "still rtl");
}

void
run_cursor_tests (void) {
  void (*functions[])() = {
    test_cursor_on_first_line,
    test_cursor_on_first_col,
    test_cursor_above_visible_window,
    test_cursor_left_of_visible_window,
    test_cursor_right_of_visible_window,
    test_cursor_in_cell_zero,
    test_cursor_not_at_row_begin,

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

    test_cursor_move_top,
    test_cursor_move_visible_top,
    test_cursor_move_bottom,
    test_cursor_move_visible_bottom,
    test_cursor_move_begin,
    test_cursor_move_end,
    test_cursor_snap_to_end,

    test_cursor_select_left,
    test_cursor_select_right,
    test_cursor_select_up,
    test_cursor_select_down,
    test_cursor_select_right_word,
    test_cursor_select_left_word,
    test_cursor_select_up_word,
    test_cursor_select_down_word,
    test_cursor_select_x_axis,
  };

  for (size_t i = 0; i < sizeof(functions) / sizeof(functions[0]); i++) {
    setup();
    functions[i]();
    teardown();
  }
}
