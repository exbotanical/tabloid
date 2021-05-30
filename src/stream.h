#include <sys/types.h>

typedef struct trow trow;

void insertRow(int at, char *s, size_t len);

void delChar(void);

void updateRow(trow *row);

void insertCharAtRow(trow *row, int at, int c);

void insertChar(int c);

void insertNewline(void);
