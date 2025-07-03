#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
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
#include "version.h"

int main(int argc, char* argv[]) {
  // Logger
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

  if (argc == 2) {
    if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
      printf("lidm version %s (git %s, build date %s)\n", LIDM_VERSION,
             LIDM_GIT_DESCRIPTION, LIDM_BUILD_DATE);
      return 0;
    } else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
      printf(
          "Usage: lidm [vt number]\n"
          "Options:\n"
          "  -h, --help     Display help menu\n"
          "  -v, --version  Display version information\n");
      return 0;
    } else {
      // Chvt
      chvt_str(argv[1]);
    }
  }

  struct config config = DEFAULT_CONFIG;
  char* conf_override = getenv("LIDM_CONF");
  char* conf_path = conf_override ? conf_override : "/etc/lidm.ini";
  if (parse_config(&config, conf_path) != 0) {
    (void)fputs("error parsing config\n", stderr);
    return 1;
  }
  setup(&config);

  struct Vector users = get_human_users();
  struct Vector sessions = get_avaliable_sessions();

  int ret = load(&users, &sessions);
  if (ret == 0) execl(argv[0], argv[0], NULL);
}
