#include "buffer.h"

#include "common.h"

#include <stdlib.h>
#include <string.h>

/**
 * @brief Takes a string buffer and appends a next string, reallocating the required memory
 *
 * @param abuf
 * @param s
 * @param len
 */
void abufAppend(struct appendBuf *abuf, const char *s, int len) {
  // get mem sizeof current str + sizeof append str
  char *next = realloc(abuf->buf, abuf->len + len);

  if (next == NULL) return;
  memcpy(&next[abuf->len], s, len);
  abuf->buf = next;
  abuf->len += len;
}

/**
 * @brief Deallocate the dynamic memory used by an `appendBuf`
 *
 * @param abuf
 */
void abufFree(struct appendBuf *abuf) {
  free(abuf->buf);
}
