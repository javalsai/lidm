#ifndef LOGH_
#define LOGH_

#include <stdio.h>

void log_init(FILE* fd);
void log_puts(const char* msg);
void log_printf(const char* fmt, ...);
// NOLINTNEXTLINE(readability-identifier-length)
void log_perror(const char* s);

#endif
