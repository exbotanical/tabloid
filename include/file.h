#ifndef FILE_H
#define FILE_H

#include <stdbool.h>
#include <unistd.h>

typedef struct {
  int fd;
  void (*read)(void);
  void (*write)(const char *fmt, ...);
  void (*open)(void);
  void (*close)(void);
} file_handle_t;

// TODO: Not portable
static bool
file_exists (const char *filepath) {
  return access(filepath, F_OK) == 0;
}

#endif /* FILE_H */
