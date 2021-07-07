/**
 * @file buffer.c
 * @author goldmund
 * @brief User input buffer management; extensible buffer configurations
 * @version 0.1
 * @date 2021-07-05
 *
 * @copyright Copyright (c) 2021 Matthew Zito (goldmund)
 *
 */

#include "buffer.h"

#include "common.h"

#include <stdlib.h>
#include <string.h>

/**
 * @brief Takes a string buffer and appends a next string, reallocating the required memory (caller must invoke `free`)
 *
 * @param e_buffer the buffer to which `s` will be appended
 * @param s char pointer to be appended to the buffer
 * @param len the length of the buffer
 */
void buf_extend(struct extensible_buffer* e_buffer, const char* s, int len) {
  // get mem sizeof current str + sizeof append str
  char* next = realloc(e_buffer->buf, e_buffer->len + len);

  if (!next) return;

  memcpy(&next[e_buffer->len], s, len);
  e_buffer->buf = next;
  e_buffer->len += len;
}

/**
 * @brief Deallocate the dynamic memory used by an `extensible_buffer`
 *
 * @param e_buffer the buffer pointer
 */
void free_e_buf(struct extensible_buffer* e_buffer) {
  free(e_buffer->buf);
}
