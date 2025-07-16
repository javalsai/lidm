#ifndef LOGH_
#define LOGH_

#include <stdio.h>

#include "macros.h"

void log_init(FILE* NNULLABLE fd);
void log_puts(const char* NNULLABLE msg);
void log_printf(const char* NNULLABLE fmt, ...);
// NOLINTNEXTLINE(readability-identifier-length)
void log_perror(const char* NNULLABLE s);

#endif
