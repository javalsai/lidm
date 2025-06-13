#ifndef DESKTOPH_
#define DESKTOPH_

#include <stdbool.h>
#include <stdio.h>

#include "macros.h"

struct status {
  bool finish;
  int ret;
};

int read_desktop(FILE* NNULLABLE fd, void* UNULLABLE ctx,
                 struct status (*NNULLABLE cb)(void* UNULLABLE ctx, char* NULLABLE table,
                                     char* NNULLABLE key,
                                     char* NNULLABLE value));

#endif
