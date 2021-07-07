/**
 * @file io.c
 * @author goldmund
 * @brief Handles file I/O to program
 * @version 0.1
 * @date 2021-07-05
 *
 * @copyright Copyright (c) 2021 Matthew Zito (goldmund)
 *
 */

/* Feature Test Macros */
#define _DEFAULT_SOURCE // ea. handles `getline` resolution
#define _BSD_SOURCE
#define _GNU_SOURCE

#include "io.h"

#include "common.h"
#include "error.h"
#include "keypress.h"
#include "render.h"
#include "stream.h"
#include "syntax.h"
#include "viewport.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// `strdup` is not yet a part of Standard C
// it is standardized in POSIX.1-2008, and may or may not be provided by <string.h>
#ifndef STRDUP_H
#define STRDUP_H

char* strdup(const char* s) {
  size_t size = strlen(s) + 1;

  char* p = malloc(size);
  if (p) {
		// from what I can tell, `memcpy` here is more efficient than `strcpy`
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
void f_open(char* filename) {
  free(T.filename);
  T.filename = strdup(filename);

	resolve_syntax();

  FILE* stream = fopen(filename, "r");
  if (!stream) panic("fopen");

  char* line = NULL;
  size_t linecap = 0; /**< Tracks how much memory has been allocated */
  ssize_t linelen;

  // -1 at EOF
  while ((linelen = getline(&line, &linecap, stream)) != -1) {
    // we know ea. `t_row` represents a single line of text, thus there is
    // no reason to store the newline, carriage return
    while (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r')) {
      linelen--;
    }

    insert_row(T.numrows, line, linelen);
  }

  free(line);
  fclose(stream);

	// because we invoke `insert_row`, status will be set to 'dirty' by default
	// mitigate w/ reset
	T.dirty = 0;
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
char* buf_out(int* buflen) {
	int totlen = 0;
	int i;

	// aggregate len as len of ea text row
	for (i = 0; i < T.numrows; i++) {
		totlen += T.row[i].size + 1;
	}

	*buflen = totlen;

	char* buf = malloc(totlen);
	char* p = buf;

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

/**
 * @brief Write editor contents to either an existing or newly created file
 *
 * @todo Write to a swapfile first
 */
void f_write(void) {
	if (!T.filename) {
		T.filename = prompt("Save as %s (ESC to cancel)", NULL);

		if (!T.filename) {
			set_stats_msg("Save cancelled");
			return;
		}

		resolve_syntax();
	}

	int len;
	char* buf = buf_out(&len);

	int fd = open(T.filename, O_RDWR | O_CREAT, 0644);

	if (fd != -1) {
		// by manually truncating to the same len as the writable data,
		// we render the overwrite safer in the case that `write` fails
		// (as opposed to passing `O_TRUNC` to `open` directly, which clears all data prior to writing)
		if (ftruncate(fd, len) != -1) {
			if (write(fd, buf, len) == len) {
					close(fd);
					free(buf);

					T.dirty = 0; // reset dirty state
					set_stats_msg("%d bytes written to disk", len);
					return;
			}
		}
		close(fd);
	}

	free(buf);
	set_stats_msg("Unable to save file; I/O error: %s", strerror(errno));
}

/**
 * @brief Prompt for and collect user input
 *
 * Expects the caller to invoke `free`
 *
 * @param prompt
 * @param cb optional callback
 * @return char*
 */
char* prompt(const char* prompt, void (*cb)(char*, int)) {
	size_t bufsize = 128;

	// store user input
	char* buf = malloc(bufsize);

	size_t buflen = 0;
	buf[0] = NULL_TERM;

	while (1) {
		set_stats_msg(prompt, buf);
		clear_screen();

		int c = readkey();

	// allow backspace in prompt
		if (c == DEL || c == CTRL_KEY('h') || c == BACKSPACE) {
			if (buflen != 0) buf[--buflen] = NULL_TERM;
		}	else if (c == ESCAPE) { // user hits esc to cancel
			set_stats_msg("");
			if (cb) cb(buf, c);
			free(buf);

			return NULL;
		} else if (c == '\r') {
			if (buflen != 0) {
				set_stats_msg("");
				if (cb) cb(buf, c);

				return buf;
			}
		} else if (!iscntrl(c) && c < 128) {
			// if `buflen` reaches max cap, double `bufsize`
			if (buflen == bufsize - 1) {
				bufsize *= 2;
				buf = realloc(buf, bufsize);
			}

			buf[buflen++] = c;
			buf[buflen] = NULL_TERM;
		}

		if (cb) cb(buf, c);
	}
}
