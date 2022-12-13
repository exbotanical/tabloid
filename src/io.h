#ifndef IO_H
#define IO_H

void f_open(char* filename);

void f_write(void);

char* status_prompt(const char* prompt, void (*cb)(char*, int));

#endif
