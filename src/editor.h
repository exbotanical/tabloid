struct appendBuf;

int readKey(void);

void clearScreen(void);

void drawRows(struct appendBuf *abuf);

void enableRawMode(void);

void disableRawMode(void);

void procKeypress(void);

void initEditor(void);
