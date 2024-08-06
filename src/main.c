#include <pwd.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#include <chvt.h>
#include <config.h>
#include <sessions.h>
#include <ui.h>
#include <users.h>

int chvt_char(char c) {
  int i = c - '0';
  if (i >= 0 && i <= 9) {
    return chvt(i);
  }
  return -1;
}

int main(int argc, char *argv[]) {
  if (argc == 2)
    chvt_char(argv[1][0]);

  struct config *config = parse_config("/etc/lidm.ini");
  if (config == NULL) {
    fprintf(stderr, "error parsing config\n");
    return 1;
  }
  setup(*config);

  struct users_list *users = get_human_users();
  struct sessions_list *sessions = get_avaliable_sessions();

  int ret = load(users, sessions);
  if (ret == 0)
    execl(argv[0], argv[0], NULL);
}
