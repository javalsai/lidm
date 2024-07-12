#ifndef _SESSIONSH_
#define _SESSIONSH_

#include <sys/types.h>

enum session_type {
  XORG,
  WAYLAND,
  SHELL,
};

struct session {
  char *name;
  char *path;
  enum session_type type;
};

struct sessions_list {
  u_int16_t length;
  struct session *sessions;
};

struct sessions_list *get_avaliable_sessions();

#endif
