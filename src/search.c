#include "search.h"

#include "common.h"
#include "io.h"
#include "viewport.h"

#include <stdlib.h>
#include <string.h>

/**
 * @brief Wraps a search driver
 *
 */
void search(void) {
	// persist original cursor pos for cases where the user aborts the search
	int saved_cx = T.curs_x;
	int saved_cy = T.curs_y;
	int saved_col = T.coloff;
	int saved_row = T.rowoff;

	char* query = prompt("Search: %s (ESC/Arrows/Enter)", eager_search);

	// != NULL; user did not abort the prompt
	if (query) {
		free(query);
	} else {
		// restore OG pos
		T.curs_x = saved_cx;
		T.curs_y = saved_cy;
		T.coloff = saved_col;
		T.rowoff = saved_row;
	}
}

/**
 * @brief Eagerly searches content text by evaluating against each query character as it is entered
 *
 * @param query
 * @param key
 */
void eager_search(char* query, int key) {
	static int last_match = -1; /**< captures the row index where the last match resides */
	static int dir = 1; /**< store the direction of the search - 1 forward, -1 backward */

	// search completed or exited
	if (key == '\r' || key == ESCAPE) {
		last_match = -1;
		dir = 1;
		return;
	// user jump to next or prev match
	} else if (key == ARR_R || key == ARR_D) {
		dir = 1;
	} else if (key == ARR_L || key == ARR_U) {
		dir = -1;
	} else {
		last_match = -1;
		// only advance when arrow keys are pressed
		// i.e. we set `dir` to forward and only backward if the user explicitly chooses to
		dir = 1;
	}

	if (last_match == -1) dir = 1;
	int curr = last_match; /**< Row index of the row currently being searched */

	int i;

	// check ea row buffer for a match
	for (i = 0; i < T.numrows; i++) {
		curr += dir;

		// support cycling through matches
		if (curr == -1) curr = T.numrows - 1;
		else if (curr == T.numrows) curr = 0;

		t_row *row = &T.row[curr];

		char* match = strstr(row->render, query);

		// if we've a match, we jump to the match pos in the viewport
		if (match) {
			last_match = curr;
			T.curs_y = curr;
			// prevent tabs overwrite by calling `ridx_to_cidx`
			T.curs_x = ridx_to_cidx(row, match - row->render);
			// scroll to very bottom; this will cause the scroller to position the match
			// at the top of the viewport
			T.rowoff = T.numrows;
			break;
		}
	}
}
