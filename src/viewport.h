#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "common.h"

void clear_screen(void);

int cidx_to_ridx(Row* row, int i_curs_x);

int ridx_to_cidx(Row* row, int rx);

int get_win_sz(int* rows, int* cols);

void curs_mv(int key);

void scroll(void);

#endif
