#include <stdlib.h>

#include "tests.h"

void
debug_curs () {
  printf("(x=%d,y=%d)\n", editor.curs.x, editor.curs.y);
  fflush(stdout);
}

void
test_cursor_setup (void) {
  editor.r        = render_state_init(NULL);
  editor.curs     = DEFAULT_CURSOR_STATE;
  editor.win.cols = 0;
  editor.win.rows = 0;
  line_pad        = 0;
}

void
test_cursor_teardown (void) {
  render_state_free(editor.r);
}

void
test_cursor_on_first_line (void) {
  editor_insert_newline();
  editor_insert_newline();
  editor_insert_newline();

  SET_CURSOR(0, 0);
  ok(cursor_on_first_line() == true, "cursor is on first line");

  SET_CURSOR(0, 1);
  ok(cursor_on_first_line() == false, "cursor is not on first line");

  SET_CURSOR(0, 2);
  ok(cursor_on_first_line() == false, "cursor is still not on first line");
}

void
test_cursor_on_first_col (void) {
  editor_insert_newline();
  editor_insert_newline();
  editor_insert_newline();

  SET_CURSOR(0, 0);
  ok(cursor_on_first_col() == true, "cursor is on first col");

  SET_CURSOR(0, 1);
  ok(cursor_on_first_col() == true, "cursor is still on first col");

  SET_CURSOR(0, 2);
  ok(cursor_on_first_col() == true, "cursor is still on first col");

  SET_CURSOR(1, 0);
  ok(cursor_on_first_col() == false, "cursor is not on first col");
}

void
test_cursor_above_visible_window (void) {
  editor.curs.row_off = 100;
  ok(cursor_above_visible_window() == true, "above visible window when row_off is > y cursor coord");

  SET_CURSOR(0, 100);
  ok(cursor_above_visible_window() == false, "not above visible window when row_off equals y cursor coord");

  SET_CURSOR(0, 101);
  ok(cursor_above_visible_window() == false, "not above visible window when row_off less than y cursor coord");
}

void
test_cursor_below_visible_window (void) {
  //   return editor.curs.y >= editor.curs.row_off + editor.win.rows;
  editor.win.rows     = 100;
  editor.curs.row_off = 1;
  SET_CURSOR(0, 102);
  ok(cursor_below_visible_window() == true, "below visible window when num rows + row_off is < y cursor coord");

  SET_CURSOR(0, 100);
  ok(cursor_below_visible_window() == false, "below visible window when num rows + row_off equals y cursor coord");

  SET_CURSOR(0, 101);
  ok(cursor_below_visible_window() == false, "not below visible window when num rows + row_off greater than y cursor coord");
}

void
test_cursor_left_of_visible_window (void) {
  editor.curs.col_off = 100;
  SET_CURSOR(99, 0);
  ok(cursor_left_of_visible_window() == true, "left of visible window when x cursor coord is less than col_off");

  SET_CURSOR(99, 5);
  ok(cursor_left_of_visible_window() == true, "left of visible window when x cursor coord is less than col_off on any line");

  SET_CURSOR(100, 0);
  ok(cursor_left_of_visible_window() == false, "not left of visible window when x cursor coord is equal to col_off");

  SET_CURSOR(101, 0);
  ok(cursor_left_of_visible_window() == false, "not left of visible window when x cursor coord is greater than col_off");
}

void
test_cursor_right_of_visible_window (void) {
  editor.curs.col_off = 10;
  editor.win.cols     = 10;
  line_pad            = 3;

  SET_CURSOR(23, 0);
  ok(cursor_right_of_visible_window() == true, "right of visible window");

  SET_CURSOR(16, 0);
  ok(cursor_right_of_visible_window() == true, "right of visible window");

  SET_CURSOR(25, 1);
  ok(cursor_right_of_visible_window() == true, "right of visible window");

  SET_CURSOR(15, 0);
  ok(cursor_right_of_visible_window() == false, "not right of visible window");
}

void
test_cursor_in_cell_zero (void) {
  SET_CURSOR(0, 0);
  ok(cursor_in_cell_zero() == true, "cursor is in cell zero");

  SET_CURSOR(1, 0);
  ok(cursor_in_cell_zero() == false, "cursor is not in cell zero");

  SET_CURSOR(0, 1);
  ok(cursor_in_cell_zero() == false, "cursor is not in cell zero");
}

void
test_cursor_not_at_row_begin (void) {
  SET_CURSOR(1, 0);
  ok(cursor_not_at_row_begin() == true, "cursor is not at row begin");

  SET_CURSOR(0, 0);
  ok(cursor_not_at_row_begin() == false, "cursor is at row begin");

  SET_CURSOR(0, 1);
  ok(cursor_not_at_row_begin() == false, "cursor is at row begin");
}

void
test_cursor_move_down (void) {
  editor_insert("hello\n");
  editor_insert("world\n");
  editor_insert("what\n");
  SET_CURSOR(1, 0);

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
  editor_insert("hello\n");
  editor_insert("hello");
  SET_CURSOR(1, 1);

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
  editor_insert("hello");
  SET_CURSOR(1, 0);

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
  editor_insert("hello\n");
  editor_insert("world\n");
  editor_insert("what\n");
  SET_CURSOR(1, 2);

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
  editor_insert("hello\n");
  editor_insert("world\n");
  editor_insert("what\n");
  SET_CURSOR(1, 0);

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
  editor_insert("what\n");
  SET_CURSOR(1, 0);

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
  editor_insert("hello\n");
  editor_insert("world\n");
  SET_CURSOR(2, 0);

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
  editor_insert("hello");
  SET_CURSOR(5, 0);

  editor_insert("\nworld");
  SET_CURSOR(0, 1);

  cursor_move_left();
  ok(editor.curs.x == 5, "moves to the end");
  ok(editor.curs.y == 0, "moves up");

  cursor_move_left();
  ok(editor.curs.x == 4, "moves left");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_left_at_begin_of_first_line (void) {
  editor_insert("hello\n");
  editor_insert("world\n");
  SET_CURSOR(0, 0);

  ok(editor.curs.x == 0, "sanity check");
  ok(editor.curs.y == 0, "sanity check");
  cursor_move_left();
  ok(editor.curs.x == 0, "can't move left");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_right (void) {
  editor_insert("hello\n");
  editor_insert("world\n");
  SET_CURSOR(2, 0);

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
  editor_insert("hello");
  SET_CURSOR(5, 0);
  editor_insert("\nworld");
  SET_CURSOR(5, 0);

  ok(editor.curs.x == 5, "sanity check");
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
  editor_insert("hello\n");
  editor_insert("world\n");
  SET_CURSOR(6, 1);

  ok(editor.curs.x == 6, "sanity check");
  ok(editor.curs.y == 1, "sanity check");
  cursor_move_right();
  ok(editor.curs.x == 6, "cannot move");
  ok(editor.curs.y == 1, "cannot move");
}

void
test_cursor_move_left_word (void) {
  editor_insert("hello world what\n");
  SET_CURSOR(17, 0);

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
  editor_insert("hello world what\n");
  SET_CURSOR(16, 0);

  ok(editor.curs.x == 16, "sanity check");
  ok(editor.curs.y == 0, "sanity check");

  cursor_move_left_word();
  ok(editor.curs.x == 12, "jumps to the beginning of the preceding word");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_left_word_no_breaks (void) {
  editor_insert("helloworldwhat\n");
  SET_CURSOR(16, 0);

  ok(editor.curs.x == 16, "sanity check");
  ok(editor.curs.y == 0, "sanity check");

  cursor_move_left_word();
  ok(editor.curs.x == 0, "jumps all the way to the beginning");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_left_word_all_breaks (void) {
  editor_insert("                \n");
  SET_CURSOR(16, 0);

  ok(editor.curs.x == 16, "sanity check");
  ok(editor.curs.y == 0, "sanity check");
  fflush(stdout);
  cursor_move_left_word();

  ok(editor.curs.x == 0, "jumps all the way to the beginning");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_left_word_at_begin_of_line (void) {
  editor_insert("hello world what");
  SET_CURSOR(16, 0);
  editor_insert("\nhello world what");
  SET_CURSOR(0, 1);

  cursor_move_left_word();
  ok(editor.curs.x == 16, "jumps to the end to the previous line");
  ok(editor.curs.y == 0, "moves to the previous line");

  cursor_move_left_word();
  ok(editor.curs.x == 12, "jumps to the beginning of the preceding word");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_left_word_at_begin_of_first_line (void) {
  editor_insert("hello world what\n");
  SET_CURSOR(0, 0);

  ok(editor.curs.x == 0, "sanity check");
  ok(editor.curs.y == 0, "sanity check");

  cursor_move_left_word();
  ok(editor.curs.x == 0, "cannot move");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_left_word_prev_break_char (void) {
  editor_insert("hello world   what\n");
  SET_CURSOR(14, 0);

  ok(editor.curs.x == 14, "sanity check");
  ok(editor.curs.y == 0, "sanity check");

  cursor_move_left_word();
  ok(editor.curs.x == 11, "jumps to end of preceding word");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_left_word_prev_non_break_char (void) {
  editor_insert("hello world   what\n");
  SET_CURSOR(11, 0);

  ok(editor.curs.x == 11, "sanity check");
  ok(editor.curs.y == 0, "sanity check");

  cursor_move_left_word();
  ok(editor.curs.x == 6, "jumps to beginning of preceding word");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_right_word (void) {
  editor_insert("hello world what");
  SET_CURSOR(0, 0);

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
  ok(editor.curs.x == 16, "jumps to the end");
  ok(editor.curs.y == 0, "does not move the y coord");

  cursor_move_right_word();
  ok(editor.curs.x == 16, "cannot move anymore");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_right_word_from_middle_of_word (void) {
  editor_insert("hello world what\n");
  SET_CURSOR(2, 0);

  ok(editor.curs.x == 2, "sanity check");
  ok(editor.curs.y == 0, "sanity check");

  cursor_move_right_word();
  ok(editor.curs.x == 5, "jumps to the end of the next word");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_right_word_no_breaks (void) {
  editor_insert("helloworldwhat");
  SET_CURSOR(0, 0);

  ok(editor.curs.x == 0, "sanity check");
  ok(editor.curs.y == 0, "sanity check");

  cursor_move_right_word();
  ok(editor.curs.x == 14, "jumps all the way to the end");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_right_word_all_breaks (void) {
  editor_insert("                \n");
  SET_CURSOR(0, 0);

  ok(editor.curs.x == 0, "sanity check");
  ok(editor.curs.y == 0, "sanity check");

  cursor_move_right_word();
  ok(editor.curs.x == 16, "jumps all the way to the end");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_right_word_at_end_of_line (void) {
  editor_insert("hello");
  SET_CURSOR(5, 0);
  editor_insert("\nworld");

  cursor_move_right_word();
  ok(editor.curs.x == 0, "moves to start of next line");
  ok(editor.curs.y == 1, "next line");

  cursor_move_right_word();
  ok(editor.curs.x == 5, "moves to end of next word");
  ok(editor.curs.y == 1, "does not move the y coord");
}

void
test_cursor_move_right_word_at_end_of_last_line (void) {
  editor_insert("hello world what\n");
  editor_insert("hello world what\n");
  SET_CURSOR(17, 1);

  ok(editor.curs.x == 17, "sanity check");
  ok(editor.curs.y == 1, "sanity check");

  cursor_move_right_word();
  ok(editor.curs.x == 17, "cannot move");
  ok(editor.curs.y == 1, "does not move the y coord");
}

void
test_cursor_move_right_word_next_break_char (void) {
  editor_insert("hello world   what\n");
  SET_CURSOR(11, 0);

  ok(editor.curs.x == 11, "sanity check");
  ok(editor.curs.y == 0, "sanity check");

  cursor_move_right_word();
  ok(editor.curs.x == 14, "jumps to beginning of next word");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_right_word_next_non_break_char (void) {
  editor_insert("hello world   what");
  SET_CURSOR(14, 0);

  cursor_move_right_word();
  ok(editor.curs.x == 18, "jumps to end of next word");
  ok(editor.curs.y == 0, "does not move the y coord");
}

void
test_cursor_move_top (void) {
  SET_CURSOR(2, 1000);
  cursor_move_top();
  ok(editor.curs.x == 2, "maintains x");
  ok(editor.curs.y == 0, "moves y to zero");

  SET_CURSOR(2, 0);
  cursor_move_top();
  ok(editor.curs.x == 2, "maintains x");
  ok(editor.curs.y == 0, "leaves y at zero");
}

void
test_cursor_move_visible_top (void) {
  editor.curs.row_off = 12;

  SET_CURSOR(2, 1000);
  cursor_move_visible_top();
  ok(editor.curs.x == 2, "maintains x");
  ok(editor.curs.y == 12, "moves y to row_off");

  SET_CURSOR(2, 12);
  cursor_move_visible_top();
  ok(editor.curs.x == 2, "maintains x");
  ok(editor.curs.y == 12, "leaves y at row_off");
}

void
test_cursor_move_bottom (void) {
  editor.win.rows = 100;

  SET_CURSOR(2, 1000);
  cursor_move_bottom();
  ok(editor.curs.x == 2, "maintains x");
  ok(editor.curs.y == 99, "moves y to numrows minus 1");

  SET_CURSOR(2, 0);
  cursor_move_bottom();
  ok(editor.curs.x == 2, "maintains x");
  ok(editor.curs.y == 99, "leaves y at numrows minus 1");
}

void
test_cursor_move_visible_bottom (void) {
  editor.r->num_lines = 100;
  editor.curs.row_off = 5;
  editor.win.rows     = 100;

  SET_CURSOR(15, 101);
  cursor_move_visible_bottom();
  ok(editor.curs.x == 15, "maintains x");
  ok(editor.curs.y == 100, "sets y to num_lines");

  SET_CURSOR(15, 100);
  cursor_move_visible_bottom();
  ok(editor.curs.x == 15, "maintains x");
  ok(editor.curs.y == 104, "sets y to row_off plus numrows minus 1");

  SET_CURSOR(15, 0);
  cursor_move_visible_bottom();
  ok(editor.curs.x == 15, "maintains x");
  ok(editor.curs.y == 104, "sets y to row_off plus numrows minus 1");
}

void
test_cursor_move_begin (void) {
  SET_CURSOR(15, 0);
  cursor_move_begin();
  ok(editor.curs.x == 0, "sets x to zero");
  ok(editor.curs.y == 0, "maintains y");

  SET_CURSOR(0, 0);
  cursor_move_begin();
  ok(editor.curs.x == 0, "keeps x at zero");
  ok(editor.curs.y == 0, "maintains y");

  SET_CURSOR(15, 5);
  cursor_move_begin();
  ok(editor.curs.x == 0, "sets x to zero");
  ok(editor.curs.y == 5, "maintains y");
}

void
test_cursor_move_end (void) {
  editor_insert("endorsed by cheneys is not a good look");

  SET_CURSOR(15, 0);
  cursor_move_end();
  ok(editor.curs.x == 38, "sets x to line length (end of row)");
  ok(editor.curs.y == 0, "maintains y");

  SET_CURSOR(0, 0);
  cursor_move_end();
  ok(editor.curs.x == 38, "sets x to line length (end of row)");
  ok(editor.curs.y == 0, "maintains y");

  SET_CURSOR(38, 5);
  cursor_move_end();
  ok(editor.curs.x == 38, "maintains x at line length (end of row)");
  ok(editor.curs.y == 5, "maintains y");

  // TODO: assert?
  SET_CURSOR(0, 500);
  cursor_move_end();
  ok(editor.curs.x == 0, "no-ops if the y coord is greater than the num of lines");
  ok(editor.curs.y == 500, "no-ops");
}

void
test_cursor_snap_to_end (void) {
  SET_CURSOR(0, 0);
  editor_insert("endorsed by cheneys is not a good look");
  SET_CURSOR(38, 0);
  editor_insert_newline();
  SET_CURSOR(0, 1);
  editor_insert("lol");

  SET_CURSOR(3, 1);
  cursor_move_up();
  cursor_snap_to_end();
  ok(editor.curs.x == 3, "maintains x when moving into a longer line");
  ok(editor.curs.y == 0, "decrements y");

  cursor_move_end();
  ok(editor.curs.x == 38, "sanity check");
  ok(editor.curs.y == 0, "sanity check");
  cursor_move_down();
  cursor_snap_to_end();
  ok(editor.curs.x == 3, "moves x to the end of the now-current line");
  ok(editor.curs.y == 1, "increments y");
}

// if (!editor.curs.select_active) {
//   editor.curs.select_anchor.x = editor.curs.x;
//   editor.curs.select_anchor.y = editor.curs.y;
// }
// editor.curs.select_active = true;

// switch (mode) {
//   case SELECT_LEFT: cursor_move_left(); break;
//   case SELECT_LEFT_WORD: cursor_move_left_word(); break;
//   case SELECT_RIGHT: cursor_move_right(); break;
//   case SELECT_RIGHT_WORD: cursor_move_right_word(); break;
// }

// editor.curs.select_offset.x = editor.curs.x;
// editor.curs.select_offset.y = editor.curs.y;

void
test_cursor_select_left (void) {
  editor_insert(
    "hello world\ngoodbye world\nhello world\ngoodbye world\nhello world"
  );
  SET_CURSOR(3, 4);
  ok(editor.curs.select_active == false, "sanity check");

  cursor_select_left();
  ok(editor.curs.select_active == true, "selects one cell to the left");
  ok(editor.curs.select_anchor.x == 3, "selects one cell to the left");
  ok(editor.curs.select_anchor.y == 4, "selects one cell to the left");
  ok(editor.curs.select_offset.x == 2, "selects one cell to the left");
  ok(editor.curs.select_offset.y == 4, "selects one cell to the left");
  ok(editor.curs.x == 2, "selects one cell to the left");
  ok(editor.curs.y == 4, "selects one cell to the left");

  cursor_select_left();
  ok(editor.curs.select_active == true, "selects one cell to the left");
  ok(editor.curs.select_anchor.x == 3, "selects one cell to the left");
  ok(editor.curs.select_anchor.y == 4, "selects one cell to the left");
  ok(editor.curs.select_offset.x == 1, "selects one cell to the left");
  ok(editor.curs.select_offset.y == 4, "selects one cell to the left");
  ok(editor.curs.x == 1, "selects one cell to the left");
  ok(editor.curs.y == 4, "selects one cell to the left");

  cursor_select_left();
  ok(editor.curs.select_active == true, "selects one cell to the left");
  ok(editor.curs.select_anchor.x == 3, "selects one cell to the left");
  ok(editor.curs.select_anchor.y == 4, "selects one cell to the left");
  ok(editor.curs.select_offset.x == 0, "selects one cell to the left");
  ok(editor.curs.select_offset.y == 4, "selects one cell to the left");
  ok(editor.curs.x == 0, "selects one cell to the left");
  ok(editor.curs.y == 4, "selects one cell to the left");

  cursor_select_left();
  ok(editor.curs.select_active == true, "selects one cell to the left");
  ok(editor.curs.select_anchor.x == 3, "selects one cell to the left");
  ok(editor.curs.select_anchor.y == 4, "selects one cell to the left");
  ok(editor.curs.select_offset.x == 13, "wraps x around");
  ok(editor.curs.select_offset.y == 3, "moves to the previous line");
  ok(editor.curs.x == 13, "selects one cell to the left");
  ok(editor.curs.y == 3, "selects one cell to the left");
  ok(cursor_is_select_ltr() == false, "is rtl");

  editor.curs.select_active = false;

  cursor_select_left();
  ok(editor.curs.select_active == true, "selects one cell to the left");
  ok(editor.curs.select_anchor.x == 13, "resets the x anchor when going active for the first time");
  ok(editor.curs.select_anchor.y == 3, "resets the y anchor when going active for the first time");
  ok(editor.curs.select_offset.x == 12, "selects one cell to the left");
  ok(editor.curs.select_offset.y == 3, "selects one cell to the left");
  ok(editor.curs.x == 12, "selects one cell to the left");
  ok(editor.curs.y == 3, "selects one cell to the left");
}

void
test_cursor_select_right (void) {
  editor_insert(
    "hello world\ngoodbye world\nhello world\ngoodbye world\nhello world"
  );
  SET_CURSOR(5, 0);
  ok(editor.curs.select_active == false, "sanity check");

  cursor_select_right();
  ok(editor.curs.select_active == true, "selects one cell to the right");
  ok(editor.curs.select_anchor.x == 5, "selects one cell to the right");
  ok(editor.curs.select_anchor.y == 0, "selects one cell to the right");
  ok(editor.curs.select_offset.x == 6, "selects one cell to the right");
  ok(editor.curs.select_offset.y == 0, "selects one cell to the right");
  ok(editor.curs.x == 6, "selects one cell to the right");
  ok(editor.curs.y == 0, "selects one cell to the right");

  run_function_n(3, cursor_select_right);
  ok(editor.curs.select_active == true, "selects 3 cells to the right");
  ok(editor.curs.select_anchor.x == 5, "selects 3 cells to the right");
  ok(editor.curs.select_anchor.y == 0, "selects 3 cells to the right");
  ok(editor.curs.select_offset.x == 9, "selects 3 cells to the right");
  ok(editor.curs.select_offset.y == 0, "selects 3 cells to the right");
  ok(editor.curs.x == 9, "selects 3 cells to the right");
  ok(editor.curs.y == 0, "selects 3 cells to the right");

  run_function_n(4, cursor_select_right);
  ok(editor.curs.select_active == true, "selects 4 cells to the right");
  ok(editor.curs.select_anchor.x == 5, "selects 4 cells to the right");
  ok(editor.curs.select_anchor.y == 0, "selects 4 cells to the right");
  ok(editor.curs.select_offset.x == 1, "wraps around to the next line");
  ok(editor.curs.select_offset.y == 1, "wraps around to the next line");
  ok(editor.curs.x == 1, "wraps around to the next line");
  ok(editor.curs.y == 1, "wraps around to the next line");

  run_function_n(13, cursor_select_right);
  ok(editor.curs.select_active == true, "selects to the right");
  ok(editor.curs.select_anchor.x == 5, "selects to the right");
  ok(editor.curs.select_anchor.y == 0, "selects to the right");
  ok(editor.curs.select_offset.x == 0, "selects to the right");
  ok(editor.curs.select_offset.y == 2, "wraps around to the next line");
  ok(editor.curs.x == 0, "selects to the right");
  ok(editor.curs.y == 2, "wraps around to the next line");

  cursor_select_clear();
  cursor_select_right();
  ok(editor.curs.select_active == true, "selects to the right");
  ok(editor.curs.select_anchor.x == 0, "selects to the right");
  ok(editor.curs.select_anchor.y == 2, "selects to the right");
  ok(editor.curs.select_offset.x == 1, "selects to the right");
  ok(editor.curs.select_offset.y == 2, "selects to the right");
  ok(editor.curs.x == 1, "selects to the right");
  ok(editor.curs.y == 2, "selects to the right");
  ok(cursor_is_select_ltr() == true, "is ltr");
}

void
test_cursor_select_up (void) {
  editor_insert(
    "hello world\ngoodbye world\nhello world\ngoodbye world\nhello world"
  );
  SET_CURSOR(3, 1);
  ok(editor.curs.select_active == false, "sanity check");

  cursor_select_up();
  ok(editor.curs.select_active == true, "select up selects all text from the anchor to the offset");
  ok(editor.curs.select_anchor.x == 3, "select up selects all text from the anchor to the offset");
  ok(editor.curs.select_anchor.y == 1, "select up selects all text from the anchor to the offset");
  ok(editor.curs.select_offset.x == 3, "select up selects all text from the anchor to the offset");
  ok(editor.curs.select_offset.y == 0, "select up selects all text from the anchor to the offset");
  ok(editor.curs.x == 3, "select up selects all text from the anchor to the offset");
  ok(editor.curs.y == 0, "select up selects all text from the anchor to the offset");
  ok(cursor_is_select_ltr() == false, "select up selects all text from the anchor to the offset");

  // Micro-feature: pressing select-up while on the first line should select the entire line to the beginning thereof
  cursor_select_up();
  ok(editor.curs.select_active == true, "select up 2x on the first line selects to the beginning of the line");
  ok(editor.curs.select_anchor.x == 3, "select up 2x on the first line selects to the beginning of the line");
  ok(editor.curs.select_anchor.y == 1, "select up 2x on the first line selects to the beginning of the line");
  ok(editor.curs.select_offset.x == 0, "select up 2x on the first line selects to the beginning of the line");
  ok(editor.curs.select_offset.y == 0, "select up 2x on the first line selects to the beginning of the line");
  ok(editor.curs.x == 0, "select up 2x on the first line selects to the beginning of the line");
  ok(editor.curs.y == 0, "select up 2x on the first line selects to the beginning of the line");
  ok(cursor_is_select_ltr() == false, "select up 2x on the first line selects to the beginning of the line");

  cursor_select_clear();
  SET_CURSOR(3, 1);
  run_function_n(3, cursor_select_right);
  cursor_select_up();
  ok(editor.curs.select_active == true, "select up maintains anchor but resets the offset");
  ok(editor.curs.select_anchor.x == 3, "select up maintains anchor but resets the offset");
  ok(editor.curs.select_anchor.y == 1, "select up maintains anchor but resets the offset");
  ok(editor.curs.select_offset.x == 6, "select up maintains anchor but resets the offset");
  ok(editor.curs.select_offset.y == 0, "select up maintains anchor but resets the offset");
  ok(editor.curs.x == 6, "select up maintains anchor but resets the offset");
  ok(editor.curs.y == 0, "select up maintains anchor but resets the offset");
  ok(cursor_is_select_ltr() == false, "select up maintains anchor but resets the offset");
}

void
test_cursor_select_down (void) {
  editor_insert(
    "hello world\ngoodbye world\nhello world\ngoodbye world\nhello world"
  );
  SET_CURSOR(3, 3);
  ok(editor.curs.select_active == false, "sanity check");

  cursor_select_down();
  ok(editor.curs.select_active == true, "select down selects all text from the anchor to the offset");
  ok(editor.curs.select_anchor.x == 3, "select down selects all text from the anchor to the offset");
  ok(editor.curs.select_anchor.y == 3, "select down selects all text from the anchor to the offset");
  ok(editor.curs.select_offset.x == 3, "select down selects all text from the anchor to the offset");
  ok(editor.curs.select_offset.y == 4, "select down selects all text from the anchor to the offset");
  ok(editor.curs.x == 3, "select down selects all text from the anchor to the offset");
  ok(editor.curs.y == 4, "select down selects all text from the anchor to the offset");
  ok(cursor_is_select_ltr() == true, "select down selects all text from the anchor to the offset");

  cursor_select_down();
  ok(editor.curs.select_active == true, "select down selects all text from the anchor to the end of the line");
  ok(editor.curs.select_anchor.x == 3, "select down selects all text from the anchor to the end of the line");
  ok(editor.curs.select_anchor.y == 3, "select down selects all text from the anchor to the end of the line");
  ok(editor.curs.select_offset.x == 11, "select down selects all text from the anchor to the end of the line");
  ok(editor.curs.select_offset.y == 4, "select down selects all text from the anchor to the end of the line");
  ok(editor.curs.x == 11, "select down selects all text from the anchor to the end of the line");
  ok(editor.curs.y == 4, "select down selects all text from the anchor to the end of the line");
  ok(cursor_is_select_ltr() == true, "select down selects all text from the anchor to the end of the line");

  cursor_select_down();
  ok(editor.curs.select_active == true, "no-ops");
  ok(editor.curs.select_anchor.x == 3, "no-ops");
  ok(editor.curs.select_anchor.y == 3, "no-ops");
  ok(editor.curs.select_offset.x == 11, "no-ops");
  ok(editor.curs.select_offset.y == 4, "no-ops");
  ok(editor.curs.x == 11, "no-ops");
  ok(editor.curs.y == 4, "no-ops");
  ok(cursor_is_select_ltr() == true, "no-ops");
}

void
test_cursor_select_right_word (void) {
  editor_insert(
    "hello world\ngoodbye world\nhello world\ngoodbye world\nhello world"
  );
  SET_CURSOR(0, 0);
  ok(editor.curs.select_active == false, "sanity check");

  cursor_select_right_word();
  ok(editor.curs.select_active == true, "selects to the right by one word");
  ok(editor.curs.select_anchor.x == 0, "selects to the right by one word");
  ok(editor.curs.select_anchor.y == 0, "selects to the right by one word");
  ok(editor.curs.select_offset.x == 5, "selects to the right by one word");
  ok(editor.curs.select_offset.y == 0, "selects to the right by one word");
  ok(editor.curs.x == 5, "selects to the right by one word");
  ok(editor.curs.y == 0, "selects to the right by one word");

  cursor_select_right_word();
  ok(editor.curs.select_active == true, "selects to the right by one word");
  ok(editor.curs.select_anchor.x == 0, "selects to the right by one word");
  ok(editor.curs.select_anchor.y == 0, "selects to the right by one word");
  ok(editor.curs.select_offset.x == 6, "selects to the right by one word");
  ok(editor.curs.select_offset.y == 0, "selects to the right by one word");
  ok(editor.curs.x == 6, "selects to the right by one word");
  ok(editor.curs.y == 0, "selects to the right by one word");

  cursor_select_right_word();
  ok(editor.curs.select_active == true, "selects to the right by one word");
  ok(editor.curs.select_anchor.x == 0, "selects to the right by one word");
  ok(editor.curs.select_anchor.y == 0, "selects to the right by one word");
  ok(editor.curs.select_offset.x == 11, "selects to the right by one word");
  ok(editor.curs.select_offset.y == 0, "selects to the right by one word");
  ok(editor.curs.x == 11, "selects to the right by one word");
  ok(editor.curs.y == 0, "selects to the right by one word");

  cursor_select_right_word();
  ok(editor.curs.select_active == true, "selects to the right by one word");
  ok(editor.curs.select_anchor.x == 0, "selects to the right by one word");
  ok(editor.curs.select_anchor.y == 0, "selects to the right by one word");
  ok(editor.curs.select_offset.x == 0, "selects to the right by one word");
  ok(editor.curs.select_offset.y == 1, "selects to the right by one word");
  ok(editor.curs.x == 0, "selects to the right by one word");
  ok(editor.curs.y == 1, "selects to the right by one word");

  cursor_select_clear();
  ok(editor.curs.select_active == false, "selects to the right by one word");
  cursor_select_right_word();
  ok(editor.curs.select_active == true, "selects to the right by one word");
  ok(editor.curs.select_anchor.x == 0, "selects to the right by one word");
  ok(editor.curs.select_anchor.y == 1, "selects to the right by one word");
  ok(editor.curs.select_offset.x == 7, "selects to the right by one word");
  ok(editor.curs.select_offset.y == 1, "selects to the right by one word");
  ok(editor.curs.x == 7, "selects to the right by one word");
  ok(editor.curs.y == 1, "selects to the right by one word");
}

void
test_cursor_select_left_word (void) {
  editor_insert(
    "hello world\ngoodbye world\nhello world\ngoodbye world\nhello world"
  );
  SET_CURSOR(7, 4);
  ok(editor.curs.select_active == false, "sanity check");

  cursor_select_left_word();
  ok(editor.curs.select_active == true, "selects to the left by one word");
  ok(editor.curs.select_anchor.x == 7, "selects to the left by one word");
  ok(editor.curs.select_anchor.y == 4, "selects to the left by one word");
  ok(editor.curs.select_offset.x == 6, "selects to the left by one word");
  ok(editor.curs.select_offset.y == 4, "selects to the left by one word");
  ok(editor.curs.x == 6, "selects to the left by one word");
  ok(editor.curs.y == 4, "selects to the left by one word");

  cursor_select_left_word();
  ok(editor.curs.select_active == true, "selects to the left by one word");
  ok(editor.curs.select_anchor.x == 7, "selects to the left by one word");
  ok(editor.curs.select_anchor.y == 4, "selects to the left by one word");
  ok(editor.curs.select_offset.x == 5, "selects to the left by one word");
  ok(editor.curs.select_offset.y == 4, "selects to the left by one word");
  ok(editor.curs.x == 5, "selects to the left by one word");
  ok(editor.curs.y == 4, "selects to the left by one word");

  cursor_select_left_word();
  ok(editor.curs.select_active == true, "selects to the left by one word");
  ok(editor.curs.select_anchor.x == 7, "selects to the left by one word");
  ok(editor.curs.select_anchor.y == 4, "selects to the left by one word");
  ok(editor.curs.select_offset.x == 0, "selects to the left by one word");
  ok(editor.curs.select_offset.y == 4, "selects to the left by one word");
  ok(editor.curs.x == 0, "selects to the left by one word");
  ok(editor.curs.y == 4, "selects to the left by one word");

  cursor_select_left_word();
  ok(editor.curs.select_active == true, "selects to the left by one word");
  ok(editor.curs.select_anchor.x == 7, "selects to the left by one word");
  ok(editor.curs.select_anchor.y == 4, "selects to the left by one word");
  ok(editor.curs.select_offset.x == 13, "selects to the left by one word");
  ok(editor.curs.select_offset.y == 3, "selects to the left by one word");
  ok(editor.curs.x == 13, "selects to the left by one word");
  ok(editor.curs.y == 3, "selects to the left by one word");

  cursor_select_left_word();
  ok(editor.curs.select_active == true, "selects to the left by one word");
  ok(editor.curs.select_anchor.x == 7, "selects to the left by one word");
  ok(editor.curs.select_anchor.y == 4, "selects to the left by one word");
  ok(editor.curs.select_offset.x == 8, "selects to the left by one word");
  ok(editor.curs.select_offset.y == 3, "selects to the left by one word");
  ok(editor.curs.x == 8, "selects to the left by one word");
  ok(editor.curs.y == 3, "selects to the left by one word");

  cursor_select_left_word();
  ok(editor.curs.select_active == true, "selects to the left by one word");
  ok(editor.curs.select_anchor.x == 7, "selects to the left by one word");
  ok(editor.curs.select_anchor.y == 4, "selects to the left by one word");
  ok(editor.curs.select_offset.x == 7, "selects to the left by one word");
  ok(editor.curs.select_offset.y == 3, "selects to the left by one word");
  ok(editor.curs.x == 7, "selects to the left by one word");
  ok(editor.curs.y == 3, "selects to the left by one word");

  cursor_select_left_word();
  ok(editor.curs.select_active == true, "selects to the left by one word");
  ok(editor.curs.select_anchor.x == 7, "selects to the left by one word");
  ok(editor.curs.select_anchor.y == 4, "selects to the left by one word");
  ok(editor.curs.select_offset.x == 0, "selects to the left by one word");
  ok(editor.curs.select_offset.y == 3, "selects to the left by one word");
  ok(editor.curs.x == 0, "selects to the left by one word");
  ok(editor.curs.y == 3, "selects to the left by one word");
}

void
test_cursor_select_up_word (void) {
  todo_start("need to implement: multi-cursor feature");
  todo_end();
}

void
test_cursor_select_down_word (void) {
  todo_start("need to implement: multi-cursor feature");
  todo_end();
}

void
test_cursor_select_x_axis (void) {
  editor_insert(
    "hello world\ngoodbye world\nhello world\ngoodbye world\nhello world"
  );
  SET_CURSOR(1, 1);
  ok(editor.curs.select_active == false, "sanity check");

  cursor_select_right();
  ok(editor.curs.select_active == true, "selects to the right");
  ok(editor.curs.select_anchor.x == 1, "selects to the right");
  ok(editor.curs.select_anchor.y == 1, "selects to the right");
  ok(editor.curs.select_offset.x == 2, "selects to the right");
  ok(editor.curs.select_offset.y == 1, "selects to the right");
  ok(editor.curs.x == 2, "selects to the right");
  ok(editor.curs.y == 1, "selects to the right");
  ok(cursor_is_select_ltr() == true, "is ltr");

  cursor_select_right_word();
  ok(editor.curs.select_active == true, "selects to the right");
  ok(editor.curs.select_anchor.x == 1, "selects to the right");
  ok(editor.curs.select_anchor.y == 1, "selects to the right");
  ok(editor.curs.select_offset.x == 7, "selects to the right");
  ok(editor.curs.select_offset.y == 1, "selects to the right");
  ok(editor.curs.x == 7, "selects to the right");
  ok(editor.curs.y == 1, "selects to the right");
  ok(cursor_is_select_ltr() == true, "is ltr");

  run_function_n(2, cursor_select_right);
  ok(editor.curs.select_active == true, "selects to the right");
  ok(editor.curs.select_anchor.x == 1, "selects to the right");
  ok(editor.curs.select_anchor.y == 1, "selects to the right");
  ok(editor.curs.select_offset.x == 9, "selects to the right");
  ok(editor.curs.select_offset.y == 1, "selects to the right");
  ok(editor.curs.x == 9, "selects to the right");
  ok(editor.curs.y == 1, "selects to the right");
  ok(cursor_is_select_ltr() == true, "is ltr");

  run_function_n(3, cursor_select_right_word);
  ok(editor.curs.select_active == true, "selects to the right");
  ok(editor.curs.select_anchor.x == 1, "selects to the right");
  ok(editor.curs.select_anchor.y == 1, "selects to the right");
  ok(editor.curs.select_offset.x == 5, "selects to the right");
  ok(editor.curs.select_offset.y == 2, "selects to the right");
  ok(editor.curs.x == 5, "selects to the right");
  ok(editor.curs.y == 2, "selects to the right");
  ok(cursor_is_select_ltr() == true, "is ltr");

  cursor_select_left();
  ok(editor.curs.select_active == true, "selects to the left");
  ok(editor.curs.select_anchor.x == 1, "selects to the left");
  ok(editor.curs.select_anchor.y == 1, "selects to the left");
  ok(editor.curs.select_offset.x == 4, "selects to the left");
  ok(editor.curs.select_offset.y == 2, "selects to the left");
  ok(editor.curs.x == 4, "selects to the left");
  ok(editor.curs.y == 2, "selects to the left");
  ok(cursor_is_select_ltr() == true, "is still ltr doe");

  cursor_select_left_word();
  ok(editor.curs.select_active == true, "selects to the left");
  ok(editor.curs.select_anchor.x == 1, "selects to the left");
  ok(editor.curs.select_anchor.y == 1, "selects to the left");
  ok(editor.curs.select_offset.x == 0, "selects to the left");
  ok(editor.curs.select_offset.y == 2, "selects to the left");
  ok(editor.curs.x == 0, "selects to the left");
  ok(editor.curs.y == 2, "selects to the left");
  ok(cursor_is_select_ltr() == true, "is still ltr doe");

  run_function_n(3, cursor_select_left);
  ok(editor.curs.select_active == true, "selects to the left");
  ok(editor.curs.select_anchor.x == 1, "selects to the left");
  ok(editor.curs.select_anchor.y == 1, "selects to the left");
  ok(editor.curs.select_offset.x == 11, "selects to the left");
  ok(editor.curs.select_offset.y == 1, "selects to the left");
  ok(editor.curs.x == 11, "selects to the left");
  ok(editor.curs.y == 1, "selects to the left");
  ok(cursor_is_select_ltr() == true, "is ltr");

  run_function_n(4, cursor_select_left_word);
  ok(editor.curs.select_active == true, "selects to the left");
  ok(editor.curs.select_anchor.x == 1, "selects to the left");
  ok(editor.curs.select_anchor.y == 1, "selects to the left");
  ok(editor.curs.select_offset.x == 11, "selects to the left");
  ok(editor.curs.select_offset.y == 0, "selects to the left");
  ok(editor.curs.x == 11, "selects to the left");
  ok(editor.curs.y == 0, "selects to the left");
  ok(cursor_is_select_ltr() == false, "now is rtl");

  cursor_select_left_word();
  ok(editor.curs.select_active == true, "selects to the left");
  ok(editor.curs.select_anchor.x == 1, "selects to the left");
  ok(editor.curs.select_anchor.y == 1, "selects to the left");
  ok(editor.curs.select_offset.x == 6, "selects to the left");
  ok(editor.curs.select_offset.y == 0, "selects to the left");
  ok(editor.curs.x == 6, "selects to the left");
  ok(editor.curs.y == 0, "selects to the left");
  ok(cursor_is_select_ltr() == false, "still rtl");
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

  for (unsigned int i = 0; i < sizeof(functions) / sizeof(functions[0]); i++) {
    test_cursor_setup();
    functions[i]();
    test_cursor_teardown();
  }
}
