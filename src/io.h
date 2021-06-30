#ifndef IO_H
#define IO_H

void f_open(char* filename);

char* prompt(const char* prompt, void (*cb)(char*, int));

void f_write(void);

#endif
