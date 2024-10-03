#ifndef DEBUG_H
#define DEBUG_H

void logger_open(void);
void logger_close(void);
void logger_read(void);
void logger_write(const char *fmt, ...);

#endif /* DEBUG_H */
