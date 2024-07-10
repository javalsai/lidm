#include <sys/types.h>

struct session {
  char *name;
  char *path;
};

struct sessions_list {
  u_int16_t length;
  struct session *sessions;
};

struct sessions_list *get_avaliable_sessions();
