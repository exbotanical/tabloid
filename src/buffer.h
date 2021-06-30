#ifndef BUFFER_H
#define BUFFER_H

struct extensible_buffer;

void buf_extend(struct extensible_buffer* e_buffer, const char* s, int len);

void free_e_buf(struct extensible_buffer* e_buffer);

#endif
