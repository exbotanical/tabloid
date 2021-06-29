#ifndef BUFFER_H
#define BUFFER_H

struct extensible_buf;

void buf_extend(struct extensible_buf *e_buffer, const char *s, int len);

void free_e_buf(struct extensible_buf *e_buffer);

#endif
