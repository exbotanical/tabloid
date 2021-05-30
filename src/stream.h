#include <sys/types.h>

typedef struct trow trow;

void appendRow(char *s, size_t len);

void delChar(void);

void updateRow(trow *row);

void insertCharAtRow(trow *row, int at, int c);

void insertChar(int c);
