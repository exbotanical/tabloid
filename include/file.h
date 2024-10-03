#ifndef FILE_H
#define FILE_H

typedef struct {
  int fd;
  void (*read)(void);
  void (*write)(const char *fmt, ...);
  void (*open)(void);
  void (*close)(void);
} file_handle_t;

#endif /* FILE_H */
