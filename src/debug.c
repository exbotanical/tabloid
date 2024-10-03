#include "debug.h"

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_LOGFILE ".log"
#define TIMESTAMP_FMT   "%Y-%m-%d %H:%M:%S"
#define LOG_HEADER      TIMESTAMP_FMT " %%s tabloid: "

#define SMALL_BUFFER    256
#define MED_BUFFER      SMALL_BUFFER * 4
#define LARGE_BUFFER    MED_BUFFER * 2

#define INVALID_FD      -1

#define LOG_GUARD             \
  if (log_fd == INVALID_FD) { \
    return;                   \
  }

char hostname[SMALL_BUFFER];

static int log_fd = INVALID_FD;

static void
set_hostname (void) {
  if (gethostname(hostname, sizeof(hostname)) == 0) {
    hostname[sizeof(hostname) - 1] = 0;
  } else {
    hostname[0] = 0;
  }
}

void
logger_read (void) {
  LOG_GUARD
}

void
logger_write (const char *fmt, ...) {
  LOG_GUARD

  va_list va;
  va_start(va, fmt);

  char         buf[LARGE_BUFFER];
  // Used to omit the header in the case of multi-line logs
  static short suppress_header = 0;

  time_t     ts                = time(NULL);
  struct tm *ts_info           = localtime(&ts);

  unsigned int buflen, headerlen = 0;
  buf[0] = 0;

  if (!suppress_header) {
    char header[SMALL_BUFFER];

    if (strftime(header, sizeof(header), LOG_HEADER, ts_info)) {
      if ((headerlen = snprintf(buf, sizeof(header), header, hostname))
          >= sizeof(header)) {
        headerlen = sizeof(header) - 1;
      }
    }
  }

  if ((buflen = vsnprintf(buf + headerlen, sizeof(buf) - headerlen, fmt, va)
                + headerlen)
      >= sizeof(buf)) {
    buflen = sizeof(buf) - 1;
  }

  write(log_fd, buf, buflen);
  // If the log doesn't end with a newline, omit the header next time (next
  // line is a continuation of that log entry)
  suppress_header = (buf[buflen - 1] != '\n');

  va_end(va);
}

void
logger_open (void) {
  if (log_fd != INVALID_FD) {
    return;
  }

  set_hostname();

  char *log_file = DEFAULT_LOGFILE;
  if ((log_fd = open(log_file, O_WRONLY | O_CREAT | O_APPEND, 0600)) < 0) {
    perror("open");
    exit(errno);
  }
}

void
logger_close (void) {
  LOG_GUARD
  close(log_fd);
}
