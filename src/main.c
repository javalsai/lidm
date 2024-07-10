#include <pwd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

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
    printf("s[%d]: %s %s %s\n", i, sessions->sessions[i].type,
           sessions->sessions[i].name, sessions->sessions[i].path);

  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  printf("lines %d\n", w.ws_row);
  printf("columns %d\n", w.ws_col);

  return 0;
}
