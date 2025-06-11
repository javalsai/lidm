#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#include "chvt.h"
#include "config.h"
#include "log.h"
#include "sessions.h"
#include "ui.h"
#include "users.h"
#include "util.h"

int main(int argc, char* argv[]) {
  char* log_output = getenv("LIDM_LOG");
  if (log_output) {
    FILE* log_fd = fopen(log_output, "w");
    if (!log_fd) {
      perror("fopen");
      (void)fputs("failed to open logfile in write mode", stderr);
      return -1;
    }

    log_init(log_fd);
  }

  if (argc == 2) chvt_str(argv[1]);

  char* conf_override = getenv("LIDM_CONF");
  struct config* config =
      parse_config(conf_override == NULL ? "/etc/lidm.ini" : conf_override);
  if (config == NULL) {
    // NOLINT(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
    (void)fputs("error parsing config\n", stderr);
    return 1;
  }
  setup(*config);

  struct Vector users = get_human_users();
  struct Vector sessions = get_avaliable_sessions();

  int ret = load(&users, &sessions);
  if (ret == 0) execl(argv[0], argv[0], NULL);
}
