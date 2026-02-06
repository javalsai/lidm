#include <errno.h>
#include <ftw.h>
#include <limits.h>
#include <spawn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "config.h"
#include "desktop.h"
#include "desktop_exec.h"
#include "log.h"
#include "macros.h"
#include "sessions.h"
#include "util/path.h"
#include "util/vec.h"

struct source_dir {
  enum SessionType type;
  char* dir;
};

#ifndef SESSIONS_XSESSIONS
  #define SESSIONS_XSESSIONS "/usr/share/xsessions"
#endif
#ifndef SESSIONS_XSESSIONS_LOCAL
  #define SESSIONS_XSESSIONS_LOCAL "/usr/local/share/xsessions"
#endif
#ifndef SESSIONS_WAYLAND
  #define SESSIONS_WAYLAND "/usr/share/wayland-sessions"
#endif
#ifndef SESSIONS_WAYLAND_LOCAL
  #define SESSIONS_WAYLAND_LOCAL "/usr/local/share/wayland-sessions"
#endif
static const struct source_dir SOURCES[] = {{XORG, SESSIONS_XSESSIONS},
                                            {XORG, SESSIONS_XSESSIONS_LOCAL},
                                            {WAYLAND, SESSIONS_WAYLAND},
                                            {WAYLAND, SESSIONS_WAYLAND_LOCAL}};

static struct Vector* cb_sessions = NULL;

struct ctx_typ {
  char* NULLABLE name;
  char* NULLABLE exec;
  char* NULLABLE tryexec;
};
struct status cb(void* _ctx, char* NULLABLE table, char* key, char* value) {
  struct ctx_typ* ctx = (struct ctx_typ*)_ctx;
  struct status ret;
  ret.finish = false;

  if (table == NULL) return ret;
  if (strcmp(table, "Desktop Entry") != 0) return ret;

  char** NULLABLE copy_at = NULL;
  if (strcmp(key, "Name") == 0) {
    if (ctx->name == NULL) copy_at = &ctx->name;
  } else if (strcmp(key, "Exec") == 0) {
    if (ctx->exec == NULL) copy_at = &ctx->exec;
  } else if (strcmp(key, "TryExec") == 0) {
    if (ctx->tryexec == NULL) copy_at = &ctx->tryexec;
  }

  if (copy_at != NULL) {
    *copy_at = strdup(value);
    if (*copy_at == NULL) {
      log_perror("strdup");
      log_puts("[E] failed to allocate memory");
      ret.finish = true;
      ret.ret = -1;
    }
  }

  if (ctx->name != NULL && ctx->exec != NULL && ctx->tryexec != NULL) {
    ret.finish = true;
    ret.ret = 0;
  }

  return ret;
}

// also, always return 0 or we will break parsing and we don't want a bad
// desktop file to break all possible sessions
static enum SessionType session_type;
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
static int fn(const char* fpath, const struct stat* sb, int typeflag) {
  UNUSED(sb);

  // guessing symlink behavior
  //  - FTW_PHYS if set doesn't follow symlinks, so ftw() has no flags and it
  //  follows symlinks, we should never get to handle that
  if (typeflag != FTW_F) return 0;
  log_printf("[I]  found file %s\n", fpath);

  struct ctx_typ ctx = {
      .name = NULL,
      .exec = NULL,
      .tryexec = NULL,
  };

  FILE* fd = fopen(fpath, "r");
  if (fd == NULL) {
    log_perror("fopen");
    log_printf("[E] error opening file '%s' for read\n", fpath);
    return 0;
  }

  int ret = read_desktop(fd, &ctx, &cb);
  // any error
  if (ret < 0) goto err_close;

  (void)fclose(fd);

  // TODO: filter based on tryexec
  // https://specifications.freedesktop.org/desktop-entry-spec/latest/recognized-keys.html
  free(ctx.tryexec);

  // just add this to the list
  if (ctx.name != NULL && ctx.exec != NULL) {
    struct session* this_session = malloc(sizeof(struct session));
    if (this_session == NULL) return 0;

    int arg_count;
    char** args;
    int parse_status = parse_exec_string(ctx.exec, &arg_count, &args);
    if (parse_status != 0 || arg_count == 0 || !args[0]) {
      log_printf("[E] parsing exec string '%s': %d\n", ctx.exec, parse_status);
      free(this_session);
      goto err_parsing;
    }
    free(ctx.exec);

    *this_session = (struct session){
        .name = ctx.name,
        .exec = session_exec_desktop(arg_count, args),
        .type = session_type,
    };

    vec_push(cb_sessions, this_session);
  }

  return 0;

err_close:
  (void)fclose(fd);
err_parsing:
  log_printf("[E] format error parsing %s", fpath);
  return 0;
}

// This code is designed to be run purely single threaded
#define LIKELY_BOUND_SESSIONS 8
struct Vector get_avaliable_sessions() {
  struct Vector sessions = VEC_NEW;
  vec_reserve(&sessions, LIKELY_BOUND_SESSIONS);

  cb_sessions = &sessions;
  for (size_t i = 0; i < LEN(SOURCES); i++) {
    log_printf("[I] parsing into %s\n", SOURCES[i].dir);
    session_type = SOURCES[i].type;
    ftw(SOURCES[i].dir, &fn, 1);
  }
  cb_sessions = NULL;

  return sessions;
}

int session_exec_exec(struct session_exec* NNULLABLE exec,
                      char* NULLABLE* NNULLABLE envp) {
  switch (exec->typ) {
    case EXEC_SHELL: {
      char* argv[] = {exec->shell, NULL};
      return execvpe(exec->shell, argv, envp);
    }
    case EXEC_DESKTOP:
      return execvpe(exec->desktop.args[0], exec->desktop.args, envp);
    default:
      __builtin_unreachable();
  }
}

/// So the reason this doesn't use the user shell is because that can really be
/// anything, also assuming it were fish for example, it can't execute posix
/// shell files and that leaves it out of most `/etc/profile.d/` scripts.
///
/// I'll just default to bash for now as it's able to source almsot everything
/// and takes most flags. Maybe will try to make this more sophisticated in the
/// future.
///
/// This respects errno. Even also works as any exec family function.
#ifndef LOGIN_SHELL
  #define LOGIN_SHELL "bash"
#endif
// This triggers login behavior
#define LOGIN_SHELL_ARG0 "-" LOGIN_SHELL
int session_exec_login_through_shell(struct session_exec* NNULLABLE exec,
                                     char* NULLABLE* NNULLABLE envp) {
  switch (exec->typ) {
    case EXEC_SHELL: {
      char* argv[] = {LOGIN_SHELL_ARG0, "-c", exec->shell, NULL};
      return execvpe(LOGIN_SHELL, argv, envp);
    }
    case EXEC_DESKTOP: {
      char* name = desktop_as_cmdline(exec->desktop.args);
      if (!name) {
        errno = EINVAL;
        return -1;
      }
      char* argv[] = {LOGIN_SHELL_ARG0, "-c", name, NULL};
      int ret = execvpe(LOGIN_SHELL, argv, envp);
      free(name);
      return ret;
    }
    default:
      __builtin_unreachable();
  }
}
