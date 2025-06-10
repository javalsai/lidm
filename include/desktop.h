#ifndef DESKTOPH_
#define DESKTOPH_

#include <stdbool.h>
#include <stdio.h>

#include "macros.h"

struct status {
  bool finish;
  int ret;
};

int read_desktop(FILE* fd, void* ctx, struct status (*cb)(void* ctx, char* NULLABLE table,
                                               char* key, char* value));

#endif
