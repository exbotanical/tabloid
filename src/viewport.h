#include <sys/types.h>

typedef struct trow trow;

void appendRow(char *s, size_t len);

void clearScreen(void);

int cursxConv(trow *row, int iCursx);

int getWindowSize(int *rows, int *cols);

void moveCursor(int key);

void scroll(void);

void updateRow(trow *row);
