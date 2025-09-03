// TODO: handle `fork() == -1`s

#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <security/pam_misc.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "auth.h"
#include "config.h"
#include "log.h"
#include "macros.h"
#include "pam.h"
#include "sessions.h"
#include "ui.h"
#include "util.h"

#define DEFAULT_XORG_DISPLAY 0
// no PATH search for now
#define XORG_COMMAND "/usr/bin/X"

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

// TODO: OR check if xorg_pid fail exited
static int wait_for_x_ready(const int xorg_pipefd[1], __pid_t xorg_pid) {
  // TODO
  UNUSED(xorg_pipefd);
  UNUSED(xorg_pid);
  sleep(2);
  return 0;
}

// TODO: properly pass this down
extern int vt;
// TODO: add error msgs
static void launch_with_xorg_server(struct session_exec* NNULLABLE exec,
                                    struct passwd* pw,
                                    char** NNULLABLE envlist) {
  int xorg_pipefd[2];
  pipe(xorg_pipefd);
  (void)fflush(NULL);
  __pid_t xorg_pid = fork();
  if (xorg_pid == 0) {
    if (!pw->pw_dir) _exit(EXIT_FAILURE);
    // !!!!!!!!!! ATTENTION: this fails silently, of course add failure msgs but
    // for now I can't so be careful
    if (vt == -1) _exit(EXIT_FAILURE);

    char* display_thing;
    asprintf(&display_thing, ":%d", DEFAULT_XORG_DISPLAY);
    if (!display_thing) _exit(EXIT_FAILURE);

    char* vt_path;
    asprintf(&vt_path, "vt%d", vt);
    if (!vt_path) {
      free(display_thing);
      _exit(EXIT_FAILURE);
    }

    // dup2(xorg_pipefd[1], STDERR_FILENO);
    // dup2(xorg_pipefd[1], STDOUT_FILENO);
    // close(xorg_pipefd[0]);
    // close(xorg_pipefd[1]);

    int exit = execle(XORG_COMMAND, XORG_COMMAND, display_thing, vt_path, NULL,
                      envlist);
    perror("exec");
    // execle("X", "X", display_thing, vt_path, "-auth", xauth_path,
    //        "-nolisten", "tcp", "-background", "none", NULL, envlist);

    printf("wtf3\n");
    (void)fflush(stdout);

    free(vt_path);
    free(display_thing);
    _exit(exit);
  }

  wait_for_x_ready(xorg_pipefd, xorg_pid);

  __pid_t xorg_session_pid = fork();
  if (xorg_session_pid == 0) {
    int exit = session_exec_exec(exec, envlist);
    perror("exec error");
    (void)fputs("failure calling session\n", stderr);
    _exit(exit);
  }

  // looks weird, waiting on -1 should wait on any child and then just check if
  // its xorg server or the session and kill the other waiting on it
  __pid_t pid;
  int status; // not even read for now
  while ((pid = waitpid(-1, &status, 0)) > 0) {
    if (pid == xorg_pid || pid == xorg_session_pid) {
      __pid_t pid_to_kill = pid ^ xorg_pid ^ xorg_session_pid;
      if (pid == xorg_pid) printf("Xorg server died\n");
      if (pid == xorg_session_pid) printf("Xorg session died\n");

      kill(pid_to_kill, SIGTERM);
      waitpid(pid_to_kill, &status, 0);
      printf("wtf %d, x%d s%d - k%d\n", status, xorg_pid, xorg_session_pid,
             pid_to_kill);
      (void)fflush(stdout);
      sleep(10);

      break;
    }
  }
}

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
                          struct session* NNULLABLE session,
                          char** NNULLABLE envlist) {
  if (chdir(pw->pw_dir) == -1) SEND_ERR("chdir");
  if (setgid(pw->pw_gid) == -1) SEND_ERR("setgid");
  if (initgroups(user, pw->pw_gid) == -1) SEND_ERR("initgroups");
  if (setuid(pw->pw_uid) == -1) SEND_ERR("setuid");

  SEND_MSG((struct child_msg){.err = false});
  DUMMY_READ();
  close(pipefd[0]);

  if (session->type == XORG) {
    launch_with_xorg_server(&session->exec, pw, envlist);
  } else {
    int exit = session_exec_exec(&session->exec, envlist);
    perror("exec error");
    (void)fputs("failure calling session\n", stderr);
    _exit(exit);
  }
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

  if (session.type == XORG) {
    char* display_env;
    asprintf(&display_env, "DISPLAY=:%d", DEFAULT_XORG_DISPLAY);
    if (!display_env) {
      print_err("alloc error");
      return false;
    }
    vec_push(&envlist_vec, display_env);
  }

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
  if (pid == 0)
    forked(pipefd, pw, user, &session, envlist);
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
