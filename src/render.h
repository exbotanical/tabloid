struct appendBuf;

void drawStatusBar(struct appendBuf *abuf);

void setStatusMessage(const char *fmt, ...);

void drawMessageBar(struct appendBuf *abuf);

void drawRows(struct appendBuf *abuf);
