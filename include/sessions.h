#ifndef SESSIONSH_
#define SESSIONSH_

#include <sys/types.h>

#include "macros.h"
#include "util.h"

enum session_type {
  XORG,
  WAYLAND,
  SHELL,
};

struct session {
  char* NNULLABLE name;
  char* NNULLABLE exec;
  char* NULLABLE tryexec;
  enum session_type type;
};

struct Vector get_avaliable_sessions();

#endif
