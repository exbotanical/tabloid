/**
 * @file search.c
 * @author goldmund
 * @brief Text search module
 * @version 0.1
 * @date 2021-07-05
 * @copyright Copyright (c) 2021 Matthew Zito (goldmund)
 *
 */

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
 * Invoked as a callback
 * @param query
 * @param key
 */
void eager_search(char* query, int key) {
	static int last_match = -1; /**< captures the row index where the last match resides */
	static int dir = 1; /**< store the direction of the search - 1 forward, -1 backward */

	// here, we persist the search match line before highlighting it
	// so we can remove the highlighting once the user has finished search mode
	static int lineno_hl; /**< store the lineno we will be restoring */
	static char* line_hl; /**< store the given line as it was before highlighting */

	// if we have a line to restore back to default color,
	// we `memcpy` it to the saved line's highlight storage and dealloc
	if (line_hl) {
		memcpy(
			T.row[lineno_hl].highlight,
			line_hl,
			T.row[lineno_hl].rsize
		);

		free(line_hl);
		line_hl = NULL;
	}
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

			// set the matched lineno that will be highlighted
			lineno_hl = curr;
			// set the original line ptr
			line_hl = malloc(row->rsize);

			// highlight the search match
			memcpy(line_hl, row->highlight, row->rsize);

			// set the query match substr to its syntax typing
			memset(
				// where idx is the idx into `render` of the given match
				&row->highlight[match - row->render],
				HL_SEARCH_MATCH,
				strlen(query)
			);

			break;
		}
	}
}
