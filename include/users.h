#ifndef _USERSH_
#define _USERSH_

#include <sys/types.h>

#include <util.h>

struct user {
  char *shell;
  char *username;
  char *display_name;
};

struct Vector get_human_users();

#endif
