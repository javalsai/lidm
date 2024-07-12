#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <users.h>
#include <util.h>

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

static const u_int8_t bs = 16;
static const u_int8_t unit_size = sizeof(struct user);

static u_int16_t alloc_size = bs;
static u_int16_t used_size = 0;

static struct user *users = NULL;
static struct users_list *__users_list = NULL;

struct users_list __list;
// This code is designed to be run purely single threaded
struct users_list *get_human_users() {
  if (users != NULL)
    return __users_list;
  else
    users = malloc(alloc_size * unit_size);

  struct passwd *pwd;
  while ((pwd = getpwent()) != NULL) {
    // practically impossible to reach this (== 0xffff)
    // but will prevent break
    if (used_size == 0xffff ||
        !(pwd->pw_dir && strncmp(pwd->pw_dir, "/home/", 6) == 0))
      continue;

    if (used_size >= alloc_size) {
      alloc_size += bs;
      users = realloc(users, alloc_size * unit_size);
    }

    users[used_size] = __new_user(pwd);
    used_size++;
  }

  users = realloc(users, used_size * unit_size);

  __list.length = used_size;
  __list.users = users;
  return __users_list = &__list;
}
