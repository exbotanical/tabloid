#ifndef STREAM_H
#define STREAM_H

#include <sys/types.h>

typedef struct t_row t_row;

void insert_row(int at, const char *const s, size_t len);

void rm_char(void);

void update_row(t_row* row);

void insert_char_at_row(t_row* row, int at, int c);

void insert_char(int c);

void insert_nl(void);

#endif
