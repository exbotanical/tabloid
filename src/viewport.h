#ifndef VIEWPORT_H
#define VIEWPORT_H

typedef struct trow trow;

void clear_screen(void);

int curs_x_conv(trow *row, int i_curs_x);

int get_win_sz(int *rows, int *cols);

void curs_mv(int key);

void scroll(void);

#endif
