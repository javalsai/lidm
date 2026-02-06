#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "chvt.h"
#include "config.h"
#include "log.h"
#include "macros.h"
#include "sessions.h"
#include "signal_handler.h"
#include "ui.h"
#include "users.h"
#include "util/vec.h"

#define DATESTR_MAXBUFSIZE 0x20

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
      // NOLINTBEGIN(clang-analyzer-deadcode.DeadStores)
      char* version = "?";
      char* revision = "?";
      char* builddate = "?";
      char* compilever = "?";
      // NOLINTEND(clang-analyzer-deadcode.DeadStores)
#ifdef LIDM_VERSION
      version = LIDM_VERSION;
#endif
#ifdef LIDM_GIT_REV
      revision = LIDM_GIT_REV;
#endif
#ifdef LIDM_BUILD_TS
      time_t ts = LIDM_BUILD_TS;
      char date_buf[DATESTR_MAXBUFSIZE];
      builddate = date_buf;
      if (strftime(date_buf, 0xff, "%Y-%m-%dT%H:%M:%SZ", localtime(&ts)) > 0)
        builddate = date_buf;
#endif
#ifdef COMPILER_VERSION
      compilever = COMPILER_VERSION;
#endif
      printf("lidm version %s (git %s, build date %s, compiler %s)\n", version,
             revision, builddate, compilever);
      return 0;
    }
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
      printf(
          "Usage: lidm [vt number]\n"
          "Options:\n"
          "  -h, --help     Display help menu\n"
          "  -v, --version  Display version information\n");
      return 0;
    }
    // Chvt
    chvt_str(argv[1]);
  }

  // Copy
  struct config config = DEFAULT_CONFIG;
  char* conf_override = getenv("LIDM_CONF");
  char* conf_path = conf_override ? conf_override : LIDM_CONF_PATH;
  if (parse_config(&config, conf_path) != 0) {
    (void)fputs("error parsing config\n", stderr);
    return 1;
  }
  setup(&config);

  struct Vector users = get_human_users();
  struct Vector sessions = get_avaliable_sessions();

  setup_sigterm();

  int ret = load(&users, &sessions);
  if (ret == 0) execl(argv[0], argv[0], NULL);
}
