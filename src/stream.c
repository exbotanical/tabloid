/**
 * @file stream.c
 * @author goldmund
 * @brief Input-to-render stream handling, row operations (intermediate I/O)
 * @version 0.1
 * @date 2021-07-05
 *
 * @copyright Copyright (c) 2021 Matthew Zito (goldmund)
 *
 */

#include "stream.h"

#include <stdlib.h>
#include <string.h>

#include "syntax.h"

/*******************
 *
 * Row Operations
 *
 ********************/

/**
 * @brief Allocate space for a new row, copy a given string at the end of the
 * row array
 *
 * @param s
 * @param len
 */
void insert_row(int at, const char* const s, size_t len) {
  if (at < 0 || at > T.num_rows) return;
  // num bytes `Row` takes * the num of desired rows
  T.row = realloc(T.row, sizeof(Row) * (T.num_rows + 1));
  // alloc mem at the specified `at` idx for the new row
  memmove(&T.row[at + 1], &T.row[at], sizeof(Row) * (T.num_rows - at));

  // update all row indices
  for (int j = at + 1; j <= T.num_rows; j++) T.row[j].idx++;

  // set current idx when row is inserted
  T.row[at].idx = at;

  T.row[at].size = len;
  T.row[at].chars = malloc(len + 1);

  memcpy(T.row[at].chars, s, len);
  T.row[at].chars[len] = NULL_TERMINATOR;

  // init tabs
  T.row[at].rsize = 0;
  T.row[at].render = NULL;

  // init highlight
  T.row[at].highlight = NULL;
  T.row[at].hl_open_comment = 0;

  update_row(&T.row[at]);

  T.num_rows++;
  T.dirty++;
}

/**
 * @brief Uses chars str of a `Row` to fill the contents of the render string
 * buffer In short, this function handles updating a row whenever its text
 * changes
 *
 * @param row
 */
void update_row(Row* row) {
  int tabs = 0;
  int j;

  // iterate chars of the row, counting tabs so as to alloc sufficient mem for
  // `render` max num chars needed for tabs is 8
  for (j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t') tabs++;
  }

  free(row->render);

  // row->size counts 1 for ea tab as it is, so we multiply by 7
  // and add to row->size to get max amt of mem needed for the rendered row
  row->render = malloc(row->size + tabs * (TAB_SIZE - 1) + 1);

  int idx = 0;  // contains num of chars copied into row->render

  // after alloc, we check whether the current char is a tab - if it is, we
  // append 1 space as ea tab must advanced the cursor forward 1 col we then
  // append spaces until we reach a tab stop i.e. a col divisible by 8
  for (j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t') {
      row->render[idx++] = ' ';
      while (idx % TAB_SIZE != 0) row->render[idx++] = ' ';
    } else {
      row->render[idx++] = row->chars[j];
    }
  }

  row->render[idx] = NULL_TERMINATOR;
  row->rsize = idx;

  syntax_highlight(row);
}

/**
 * @brief Insert a new character into a given tty row at a given position
 *
 * @param row
 * @param at
 * @param c
 */
void insert_char_at_row(Row* row, int at, int c) {
  // validate `at` (the idx we are inserting into)
  // if 1 char past end of str, char inserted at end
  if (at < 0 || at > row->size) at = row->size;

  // alloc mem for the chars of the `Row`, null byte
  row->chars = realloc(row->chars, row->size + 2);
  // alloc for new char
  memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
  row->size++;

  row->chars[at] = c;
  // ensure `render_x` `rsize` are updated
  update_row(row);

  T.dirty++;
}

/**
 * @brief Remove a character from a given position of a given row
 *
 * @param row
 * @param at
 */
void rm_char_at_row(Row* row, int at) {
  if (at < 0 || at >= row->size) return;

  // overwrite the deleted char w/ the chars that succeed it
  memmove(&row->chars[at], &row->chars[at + 1], row->size - at);

  row->size--;
  update_row(row);
  T.dirty++;
}

/**
 * @brief Free memory owned by a deleted row
 *
 * @param row
 */
void free_row(Row* row) {
  free(row->render);
  free(row->chars);
  free(row->highlight);
}

/**
 * @brief Delete row by overwriting the deleted row struct with rows that
 * succeed it
 *
 * @param at
 */
void rm_row(int at) {
  if (at < 0 || at > T.num_rows) {
    return;
  }

  free_row(&T.row[at]);
  memmove(&T.row[at], &T.row[at + 1], sizeof(Row) * (T.num_rows - at - 1));

  // update all row indices
  for (int j = at; j < T.num_rows - 1; j++) T.row[j].idx--;

  T.num_rows--;
  T.dirty++;
}

/**
 * @brief Append a string to that of another row
 *
 * @param row
 * @param s
 * @param len
 */
void ap_str_to_row(Row* row, char* s, size_t len) {
  row->chars = realloc(row->chars, row->size + len + 1);

  memcpy(&row->chars[row->size], s, len);
  row->size += len;
  row->chars[row->size] = NULL_TERMINATOR;
  update_row(row);
  T.dirty++;
}

/**
 * @brief Insert a newline, handles ENTER key op
 *
 */
void insert_nl(void) {
  // begin line, just insert a new row before curr line
  if (T.curs_x == 0) {
    insert_row(T.curs_y, "", 0);
  } else {
    // prep to split current line into two rows
    Row* row = &T.row[T.curs_y];

    // contents of curr row to right of cursor
    insert_row(T.curs_y + 1, &row->chars[T.curs_x], row->size - T.curs_x);

    // reset pointer (`insert_row` calls `realloc` and may move memory,
    // invalidating it)
    row = &T.row[T.curs_y];

    // truncate curr row to size and pos of the cursor
    row->size = T.curs_x;
    row->chars[row->size] = NULL_TERMINATOR;
    update_row(row);
  }

  // move to new row begin
  T.curs_y++;
  T.curs_x = 0;
}

/******************************
 *
 * Input / Character Operations
 *
 *******************************/

/**
 * @brief Wrapper, inserts character into a row
 *
 * @param c
 */
void insert_char(int c) {
  // if cursor is line after EOD, append new row prior to inserting
  if (T.curs_y == T.num_rows) {
    insert_row(T.num_rows, "", 0);
  }

  insert_char_at_row(&T.row[T.curs_y], T.curs_x, c);
  T.curs_x++;
}

/**
 * @brief Wrapper, deletes character from a row
 *
 */
void rm_char(void) {
  // if we're past EOF, return
  if (T.curs_y == T.num_rows) {
    return;
  }
  // if we're at line begin, return
  if (T.curs_x == 0 && T.curs_y == 0) {
    return;
  }

  // fetch row of cursor pos
  Row* row = &T.row[T.curs_y];

  if (T.curs_x > 0) {
    rm_char_at_row(row, T.curs_x - 1);
    T.curs_x--;
  } else {
    // set cursor to end of contents on prev row
    T.curs_x = T.row[T.curs_y - 1].size;
    ap_str_to_row(&T.row[T.curs_y - 1], row->chars, row->size);
    rm_row(T.curs_y);
    T.curs_y--;
  }
}
