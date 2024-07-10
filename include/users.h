#include <sys/types.h>

struct user {
  char *username;
  char *display_name;
};

struct users_list {
  u_int16_t length;
  struct user *users;
};

struct users_list *get_human_users();
