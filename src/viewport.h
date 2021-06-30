#ifndef VIEWPORT_H
#define VIEWPORT_H

typedef struct t_row t_row;

void clear_screen(void);

int cidx_to_ridx(t_row* row, int i_curs_x);

int ridx_to_cidx(t_row* row, int rx);

int get_win_sz(int* rows, int* cols);

void curs_mv(int key);

void scroll(void);

#endif
