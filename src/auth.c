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
#include "log.h"
#include "pam.h"
#include "sessions.h"
#include "ui.h"
#include "util.h"

static void try_source_file(struct Vector* NNULLABLE vec_envlist,
                     char* NNULLABLE filepath) {
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

    for (ssize_t i = 1; i < read; i++) {
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

static void source_paths(struct Vector* NNULLABLE vec_envlist,
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
      free(path);
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
#define DUMMY_READ()                          \
  {                                           \
    char _dummy;                              \
    read(pipefd[0], &_dummy, sizeof(_dummy)); \
  }
inline static void forked(int pipefd[2], struct passwd* pw,
                          char* NNULLABLE user,
                          struct session_exec* NNULLABLE exec,
                          char** NNULLABLE envlist) {
  if (chdir(pw->pw_dir) == -1) SEND_ERR("chdir");
  if (setgid(pw->pw_gid) == -1) SEND_ERR("setgid");
  if (initgroups(user, pw->pw_gid) == -1) SEND_ERR("initgroups");
  if (setuid(pw->pw_uid) == -1) SEND_ERR("setuid");

  // or maybe Xorg fork should happen here

  SEND_MSG((struct child_msg){.err = false});
  DUMMY_READ();
  close(pipefd[0]);

  int exit = session_exec_exec(exec, envlist);
  perror("exec error");
  (void)fputs("failure calling session\n", stderr);
  _exit(exit);
}
#undef SEND_MSG
#undef SEND_ERR
#undef DUMMY_READ

// NOLINTBEGIN(readability-function-cognitive-complexity)
bool launch(char* user, char* passwd, struct session session, void (*cb)(void),
            struct config* config) {
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
      pamh_get_complete_env(pamh, pw, session.type);
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

  // TODO: start X server here if needed
  // e.g. spawn (also after downgrading privs):
  //
  // `X :0 tty<X> -auth <user-home>/.Xauthority -nolisten tcp -background none`
  //
  // Then `DISPLAY=:0 <xsession>`

  int pipefd[2];
  pipe(pipefd);

  uint pid = fork();
  if (pid == 0)
    forked(pipefd, pw, user, &session.exec, envlist);
  else {
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
