#include <errno.h>
#include <limits.h>
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

int chvt_str(char *str) {
  char *err;
  long i = strtol(str, &err, 10);
  if (errno) {
    perror("strol");
    return -1;
  }
  // I'm not gonna elaborate on this....
  if (i > INT_MAX || i < INT_MIN || *err)
    return -1;

  return chvt((int)i);
}

int main(int argc, char *argv[]) {
  if (argc == 2)
    chvt_str(argv[1]);

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
