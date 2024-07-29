#include <pwd.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#include <config.h>
#include <sessions.h>
#include <ui.h>
#include <users.h>

void chvt(char *arg) {
  size_t bsize = snprintf(NULL, 0, "chvt %s", arg) + 1;
  char *buf = malloc(bsize);
  snprintf(buf, bsize, "chvt %s", arg);
  system(buf);
  free(buf);
}

int main(int argc, char *argv[]) {
  if (argc == 2)
    chvt(argv[1]);

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
