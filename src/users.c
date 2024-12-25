#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "users.h"
#include "util.h"

static struct user __new_user(struct passwd *p) {
  struct user __user;
  strcln(&__user.shell, p->pw_shell);
  strcln(&__user.username, p->pw_name);
  if (p->pw_gecos[0] == '\0')
    __user.display_name = __user.username;
  else
    strcln(&__user.display_name, p->pw_gecos);

  return __user;
}

// This code is designed to be run purely single threaded
struct Vector get_human_users() {
  struct Vector users = vec_new();

  struct passwd *pwd;
  while ((pwd = getpwent()) != NULL) {
    if (!(pwd->pw_dir && strncmp(pwd->pw_dir, "/home/", 6) == 0))
      continue;

    struct user *user_i = malloc(sizeof(struct user));
    *user_i = __new_user(pwd);
    vec_push(&users, user_i);
  }

  return users;
}
