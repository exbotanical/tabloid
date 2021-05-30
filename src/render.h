struct appendBuf;

void drawMessageBar(struct appendBuf *abuf);

void drawRows(struct appendBuf *abuf);

void drawStatusBar(struct appendBuf *abuf);

void setStatusMessage(const char *fmt, ...);
