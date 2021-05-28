struct appendBuf;

void abufAppend(struct appendBuf *abuf, const char *s, int len);

void abufFree(struct appendBuf *abuf);
