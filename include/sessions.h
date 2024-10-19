#ifndef _SESSIONSH_
#define _SESSIONSH_

#include <sys/types.h>

#include <util.h>

enum session_type {
  XORG,
  WAYLAND,
  SHELL,
};

struct session {
  char *name;
  char *exec;
  char *tryexec;
  enum session_type type;
};

struct Vector get_avaliable_sessions();

#endif
