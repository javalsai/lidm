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

int main(int argc, char *argv[]) {
  if (argc == 2)
    chvt_str(argv[1]);

  char* conf_override = getenv("LIDM_CONF");
  struct config *config = parse_config(conf_override == NULL ? "/etc/lidm.ini" : conf_override);
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
