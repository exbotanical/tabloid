struct appendBuf;
typedef struct trow trow;

int readKey(void);

void clearScreen(void);

void updateRow(trow *row);

void enableRawMode(void);

void disableRawMode(void);

void procKeypress(void);

void initEditor(void);

void editorOpen(char *filename);

void scroll(void);

int cursxConv(trow *row, int iCursx);
