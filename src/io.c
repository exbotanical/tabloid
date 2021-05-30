/* Feature Test Macros */
#define _DEFAULT_SOURCE // ea. handles `getline` resolution
#define _BSD_SOURCE
#define _GNU_SOURCE

#include "io.h"

#include "common.h"
#include "error.h"
#include "stream.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// `strdup` is not yet a part of Standard C
// it is standardized in POSIX.1-2008, and may or may not be provided by <string.h>
#ifndef STRDUP_H
#define STRDUP_H

char *strdup(const char *s) {
  size_t size = strlen(s) + 1;
  char *p = malloc(size);
  if (p != NULL) {
    memcpy(p, s, size);
  }
  return p;
}

#endif

/***********
 *
 * File I/O
 *
 ***********/

/**
 * @brief Open a file in the editor
 *
 * @param filename
 */
void editorOpen(char *filename) {
  free(T.filename);
  T.filename = strdup(filename);

  FILE *fp = fopen(filename, "r");
  if (!fp) panic("fopen");

  char *line = NULL;
  size_t linecap = 0; /**< Tracks how much memory has been allocated */
  ssize_t linelen;

  // -1 at EOF
  while ((linelen = getline(&line, &linecap, fp)) != -1) {
    // we know ea. `trow` represents a single line of text, thus there is
    // no reason to store the newline, carriage return
    while (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r')) {
      linelen--;
    }

    appendRow(line, linelen);
  }

  free(line);
  fclose(fp);
}

/**
 * @brief Convert the current editor contents into a string
 *
 * Prepare the current editor contents for writing to a file;
 * aggregates the length of ea row of text as `buflen`.
 * Copies ea row content to the end of the buffer, inserting newlines.
 *
 * Expects the caller to invoke `free`
 *
 * @param buflen
 * @return char*
 */
char *strConvRows(int *buflen) {
	int totlen = 0;
	int i;

	// aggregate len as len of ea text row
	for (i = 0; i < T.numrows; i++) {
		totlen += T.row[i].size + 1;
	}

	*buflen = totlen;

	char *buf = malloc(totlen);
	char *p = buf;

	// copy ea text row into buffer
	for (i = 0; i < T.numrows; i++) {
		memcpy(p, T.row[i].chars, T.row[i].size);
		p += T.row[i].size;
		// manual NL insertion because we aren't parsing these
		*p = '\n';
		p++;
	}

	return buf;
}
