struct appendBuf;
typedef struct trow trow;

int readKey(void);

void clearScreen(void);

void drawRows(struct appendBuf *abuf);

void updateRow(trow *row);

void enableRawMode(void);

void disableRawMode(void);

void procKeypress(void);

void initEditor(void);

void editorOpen(char *filename);

void scroll(void);

int cursxConv(trow *row, int iCursx);

void drawStatusBar(struct appendBuf *abuf);

void setStatusMessage(const char *fmt, ...);

void drawMessageBar(struct appendBuf *abuf);
