#include <errno.h>
#include <stdarg.h>
#include <string.h>

#include "log.h"

static FILE* logger_out = NULL;

void log_init(FILE* fd) {
  if (logger_out) (void)fclose(logger_out);
  logger_out = fd;
}

void log_puts(const char* msg) {
  if (!logger_out) return;
  (void)fputs(msg, logger_out);
}

void log_printf(const char* fmt, ...) {
  if (!logger_out) return;

  va_list args;
  va_start(args, fmt);

  (void)vfprintf(logger_out, fmt, args);

  va_end(args);
}

// NOLINTNEXTLINE(readability-identifier-length)
void log_perror(const char* s) {
  if (!logger_out) return;

  if (s)
    (void)fprintf(logger_out, "%s: %s\n", s, strerror(errno));
  else
    (void)fprintf(logger_out, "%s\n", strerror(errno));
}
