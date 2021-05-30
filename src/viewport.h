typedef struct trow trow;

void clearScreen(void);

int cursxConv(trow *row, int iCursx);

int getWindowSize(int *rows, int *cols);

void moveCursor(int key);

void scroll(void);
