#ifndef VIEWPORT_H
#define VIEWPORT_H

typedef struct t_row t_row;

void clear_screen(void);

int curs_x_conv(t_row* row, int i_curs_x);

int get_win_sz(int* rows, int* cols);

void curs_mv(int key);

void scroll(void);

#endif
