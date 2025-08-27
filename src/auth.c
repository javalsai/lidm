#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <security/pam_misc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#include "auth.h"
#include "config.h"
#include "desktop_exec.h"
#include "log.h"
#include "pam.h"
#include "sessions.h"
#include "ui.h"
#include "util.h"

void try_source_file(struct Vector* NNULLABLE vec_envlist, char* filepath) {
  log_printf("sourcing %s\n", filepath);
  FILE* file2source = fopen(filepath, "r");
  if (file2source == NULL) {
    log_printf("error sourcing %s\n", filepath);
    return;
  }

  char* line = NULL;
  size_t len = 0;
  ssize_t read;

  while ((read = getline(&line, &len, file2source)) != -1) {
    if (read == 0 || (read > 0 && *line == '#')) continue;
    if (line[read - 1] == '\n') line[read - 1] = '\0';

    for (size_t i = 1; i < read; i++) {
      if (line[i] == '=') {
        vec_push(vec_envlist, (void*)line);
        line = NULL;
        break;
      }
    }
  }

  if (line) free(line);
  (void)fclose(file2source);
}

void source_paths(struct Vector* NNULLABLE vec_envlist,
                  struct Vector* NNULLABLE abs_source,
                  const char* NULLABLE user_home,
                  struct Vector* NNULLABLE user_source) {
  for (size_t i = 0; i < abs_source->length; i++) {
    char* path = vec_get(abs_source, i);
    try_source_file(vec_envlist, path);
  }

  if (user_home)
    for (size_t i = 0; i < user_source->length; i++) {
      char* path = NULL;
      asprintf(&path, "%s/%s", user_home, (char*)vec_get(user_source, i));
      if (!path) {
        log_puts("alloc failure on user source\n");
        continue;
      }
      try_source_file(vec_envlist, path);
    }
  else {
    log_puts("user has no home\n");
  }
}

/*char *buf;*/
/*size_t bsize = snprintf(NULL, 0, "/run/user/%d", pw->pw_uid) + 1;*/
/*buf = malloc(bsize);*/
/*snprintf(buf, bsize, "/run/user/%d", pw->pw_uid);*/
/*setenv("XDG_RUNTIME_DIR", buf, true);*/
/*setenv("XDG_SESSION_CLASS", "user", true);*/
/*setenv("XDG_SESSION_ID", "1", true);*/
/*setenv("XDG_SESSION_DESKTOP", , true);*/
/*setenv("XDG_SEAT", "seat0", true);*/

struct child_msg {
  char* msg;
  int _errno;
  bool err;
};

// NOLINTBEGIN(readability-function-cognitive-complexity)
bool launch(char* user, char* passwd, struct session session, void (*cb)(void),
            struct config* config) {
  char** desktop_exec;
  int desktop_count;

  if (session.type != SHELL) {
    desktop_exec = NULL;
    int parse_status =
        parse_exec_string(session.exec, &desktop_count, &desktop_exec);
    if (parse_status != 0 || desktop_count == 0 || !desktop_exec[0]) {
      print_err("failure parsing exec string");
      log_printf("failure parsing exec string '%s': %d\n",
                 session.exec ? session.exec : "NULL", parse_status);
      free_parsed_args(desktop_count, desktop_exec);
      return false;
    }
  }

  struct passwd* pw = getpwnam(user);
  if (pw == NULL) {
    print_err("could not get user info");
    return false;
  }

  pam_handle_t* pamh = get_pamh(user, passwd);
  if (pamh == NULL) {
    print_err("error on pam authentication");
    return false;
  }

  struct pamh_getenv_status env_ret =
      pamh_get_complete_env(pamh, user, pw, session.type);
  if (env_ret.error_flag != PAMH_ERR_NOERR) {
    if (env_ret.error_flag == PAMH_ERR_ALLOC) {
      print_err("allocator error");
    } else if (env_ret.error_flag == PAMH_ERR_ERRNO) {
      print_errno(env_ret.errfn);
    } else if (env_ret.error_flag == PAMH_ERR_NOERRNO) {
      print_err(env_ret.errfn);
    }
    return false;
  }

  struct Vector envlist_vec = vec_from_raw((void**)env_ret.envlist);
  source_paths(&envlist_vec, &config->behavior.source, pw->pw_dir,
               &config->behavior.user_source);
  char** envlist = (char**)vec_as_raw(envlist_vec);
  if (!envlist) {
    print_err("vec alloc error");
    return false;
  }

  int pipefd[2];
  pipe(pipefd);

  uint pid = fork();
  if (pid == 0) { // child

#define SEND_MSG(MSG)                                   \
  {                                                     \
    write(pipefd[1], &(MSG), sizeof(struct child_msg)); \
    close(pipefd[1]);                                   \
  }
#define SEND_ERR(MSG)                                                      \
  {                                                                        \
    write(pipefd[1],                                                       \
          &(struct child_msg){.msg = (MSG), ._errno = errno, .err = true}, \
          sizeof(struct child_msg));                                       \
    close(pipefd[1]);                                                      \
    _exit(EXIT_FAILURE);                                                   \
  }

    if (chdir(pw->pw_dir) == -1) SEND_ERR("chdir");
    if (setgid(pw->pw_gid) == -1) SEND_ERR("setgid");
    if (initgroups(user, pw->pw_gid) == -1) SEND_ERR("initgroups");
    if (setuid(pw->pw_uid) == -1) SEND_ERR("setuid");

    SEND_MSG((struct child_msg){.err = false});
#undef SEND_MSG
#undef SEND_ERR
    char _;
    read(pipefd[0], &_, sizeof(_));
    close(pipefd[0]);

    int exit;
    if (session.type == SHELL) {
      exit = execle(session.exec, session.exec, NULL, envlist);
    } else if (session.type == XORG || session.type == WAYLAND) {
      // TODO: test existence of executable with TryExec
      // NOLINTNEXTLINE
      exit = execve(desktop_exec[0], desktop_exec, envlist);
      // NOLINTNEXTLINE
      free_parsed_args(desktop_count, desktop_exec);
    } else
      exit = -1;
    perror("exec error");
    (void)fputs("failure calling session\n", stderr);
    _exit(exit);
  } else {
    struct child_msg msg;
    read(pipefd[0], &msg, sizeof(struct child_msg));
    close(pipefd[0]);
    if (msg.err) {
      errno = msg._errno;
      print_errno(msg.msg);
      return false;
    }

    if (cb) cb();
    printf("\x1b[0m\x1b[H\x1b[J");
    (void)fflush(stdout);
    close(pipefd[1]);

    int exit_code;
    waitpid((pid_t)pid, &exit_code, 0);

    pam_setcred(pamh, PAM_DELETE_CRED);
    pam_close_session(pamh, 0);
    pam_end(pamh, PAM_SUCCESS);

    if (exit_code != 0) return false;
    exit(0);
  }

  return true;
}
// NOLINTEND(readability-function-cognitive-complexity)
