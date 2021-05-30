typedef struct trow trow;

void clearScreen(void);

void updateRow(trow *row);

void scroll(void);

int cursxConv(trow *row, int iCursx);

void moveCursor(int key);

int getWindowSize(int *rows, int *cols);

void appendRow(char *s, size_t len);
