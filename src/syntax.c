#include "syntax.h"

#include "common.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Read in a row and for each character, set its highlight / syntax type
 *
 * @param row
 */
void highlight_syntax(t_row *row) {
	// where `row.highlight` will either be a NULL pointer
	// or larger than when last passed in

	// we use `rsize` because both arrays are synced
	// and thereby have the same size
	row->highlight = realloc(row->highlight, row->rsize);
	memset(row->highlight, HL_DEFAULT, row->rsize);

	int i;
	for (i = 0; i < row->rsize; i++) {
		if (isdigit(row->render[i])) {
			row->highlight[i] = HL_NUMBER;
		}
	}
}

/**
 * @brief
 *
 * @param hl
 * @return int
 *
 * @todo read configuration file for color settings
 */
int map_syntax_to_color(int hl) {
	switch (hl) {
		case HL_NUMBER: return 31;
		case HL_SEARCH_MATCH: return 34;
		default: return 37;
	}
}
