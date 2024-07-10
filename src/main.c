#include <pwd.h>
#include <stdio.h>
#include <sys/types.h>

#include <sessions.h>
#include <users.h>

int main() {
  struct users_list *users = get_human_users();
  struct sessions_list *sessions = get_avaliable_sessions();

  printf("users(%hu) sessions(%hu)\n", users->length, sessions->length);

  for (uint i = 0; i < users->length; i++)
    printf("u[%d]: %s %s\n", i, users->users[i].username,
           users->users[i].display_name);

  for (uint i = 0; i < sessions->length; i++)
    printf("s[%d]: %s %s\n", i, sessions->sessions[i].name,
           sessions->sessions[i].path);

  return 0;
}
