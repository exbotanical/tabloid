#ifndef STREAM_H
#define STREAM_H

#include <sys/types.h>

typedef struct trow trow;

void insert_row(int at, char *s, size_t len);

void rm_char(void);

void update_row(trow *row);

void insert_char_at_row(trow *row, int at, int c);

void insert_char(int c);

void insert_nl(void);

#endif
