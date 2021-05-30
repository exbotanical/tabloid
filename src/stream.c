#include "stream.h"

#include "common.h"

#include <string.h>
#include <stdlib.h>

/*******************
*
* Row Operations
*
********************/

/**
 * @brief Allocate space for a new row, copy a given string at the end of the row array
 *
 * @param s
 * @param len
 */
void appendRow(char *s, size_t len) {
  // num bytes `trow` takes * the num of desired rows
  T.row = realloc(T.row, sizeof(trow) * (T.numrows + 1));

  // idx of new row to init
  int at = T.numrows;

  T.row[at].size = len;
  T.row[at].chars = malloc(len + 1);

  memcpy(T.row[at].chars, s, len);
  T.row[at].chars[len] = '\0';

  // init tabs
  T.row[at].rsize = 0;
  T.row[at].render = NULL;
  updateRow(&T.row[at]);

  T.numrows++;
}

/**
 * @brief USes chars str of a `trow` to fill the contents of the render string buffer
 *
 * @param row
 */
void updateRow(trow *row) {
  int tabs = 0;
  int j;

  // iterate chars of the row, counting tabs so as to alloc sufficient mem for `render`
  // max num chars needed for tabs is 8
  for (j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t') tabs++;
  }

  free(row->render);

  // row->size counts 1 for ea tab as it is, so we multiply by 7
  // and add to row->size to get max amt of mem needed for the rendered row
  row->render = malloc(row->size + tabs * (TAB_SIZE - 1) + 1);

  int idx = 0; // contains num of chars copied into row->render

  // after alloc, we check whether the current char is a tab - if it is, we append 1 space
  // as ea tab must advanced the cursor forward 1 col
  // we then append spaces until we reach a tab stop i.e. a col divisible by 8
  for (j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t') {
      row->render[idx++] = ' ';
      while (idx % TAB_SIZE != 0) row->render[idx++] = ' ';
    } else {
      row->render[idx++] = row->chars[j];
    }
  }

  row->render[idx] = '\0';
  row->rsize = idx;
}

/**
 * @brief Insert a new character into a given tty row at a given position
 *
 * @param row
 * @param at
 * @param c
 */
void insertCharAtRow(trow *row, int at, int c) {
	// validate `at` (the idx we are inserting into)
	// if 1 char past end of str, char inserted at end
	if (at < 0 || at > row->size) at = row->size;

	// alloc mem for the chars of the `trow`, null byte
	row->chars = realloc(row->chars, row->size + 2);
	// alloc for new char
	memmove(
		&row->chars[at + 1],
		&row->chars[at],
		row->size - at + 1
	);
	row->size++;

	row->chars[at] = c;
	// ensure `renderx` `rsize` are updated
	updateRow(row);
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
void insertChar(int c) {
	// if cursor is line after EOD, append new row prior to inserting
	if (T.cursy == T.numrows) {
		appendRow("", 0);
	}

	insertCharAtRow(&T.row[T.cursy], T.cursx, c);
	T.cursx++;
}
