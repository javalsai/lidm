#ifndef SESSIONSH_
#define SESSIONSH_

#include <sys/types.h>

#include "macros.h"
#include "util.h"

enum SessionType {
  XORG,
  WAYLAND,
  SHELL,
};

struct session {
  char* NNULLABLE name;
  char* NNULLABLE exec;
  char* NULLABLE tryexec;
  enum SessionType type;
};

struct Vector get_avaliable_sessions();

#endif
