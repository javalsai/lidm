#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "log.h"
#include "macros.h"
#include "users.h"
#include "util.h"

// NOLINTNEXTLINE(readability-identifier-length)
int build_user(struct user* NNULLABLE user, struct passwd* p) {
  char* shell = strdup(p->pw_shell);
  char* username = strdup(p->pw_name);
  char* displayname;
  if (strlen(p->pw_gecos) == 0)
    displayname = username;
  else {
    displayname = strdup(p->pw_gecos);
  }
  if (!shell || !username || !displayname) {
    free(shell);
    free(username);
    if (displayname != username) free(displayname);
    // actually tidy struggles with the line i have avobe
    // NOLINTNEXTLINE(clang-analyzer-unix.Malloc)
    return -1;
  }

  *user = (struct user){
      .shell = shell,
      .username = username,
      .display_name = displayname,
  };

  return 0;
}

// This code is designed to be run purely single threaded
// NOLINTNEXTLINE(modernize-macro-to-enum)
#define LIKELY_BOUND_USERS 4
struct Vector get_human_users() {
  struct Vector users = vec_new();
  vec_reserve(&users, LIKELY_BOUND_USERS);

  struct passwd* NULLABLE pwd;
  while ((pwd = getpwent()) != NULL) {
    log_printf("[I] handling user %s\n", pwd->pw_name);
    // `- 1` bcs sizeof counts the nullbyte
    if (pwd->pw_dir &&
        strncmp(pwd->pw_dir, "/home/", sizeof("/home/") - 1) != 0)
      continue;
    log_printf("[I]  found %s\n", pwd->pw_name);

    struct user* user_i = malloc(sizeof(struct user));
    if (user_i == NULL) continue; // malloc error

    if (build_user(user_i, pwd) != 0) {
      log_printf("[E] failed to allocate allocate memory for %s\n",
                 pwd->pw_name);
    }
    vec_push(&users, user_i);
  }

  return users;
}
