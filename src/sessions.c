#include <ftw.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "desktop.h"
#include "log.h"
#include "sessions.h"
#include "util.h"

struct source_dir {
  enum session_type type;
  char* dir;
};
static const struct source_dir sources[] = {
    {XORG, "/usr/share/xsessions"},
    {WAYLAND, "/usr/share/wayland-sessions"},
};

static struct Vector* cb_sessions = NULL;

struct ctx_typ {
  char* NULLABLE name;
  char* NULLABLE exec;
  char* NULLABLE tryexec;
};
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
struct status cb(void* _ctx, char* NULLABLE table, char* key, char* value) {
  struct ctx_typ* ctx = (struct ctx_typ*)_ctx;
  struct status ret;
  ret.finish = false;

  if (table == NULL) return ret;
  if (strcmp(table, "[Desktop Entry]") != 0) return ret;

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
static enum session_type session_type;
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
static int fn(const char* fpath, const struct stat* sb, int typeflag) {
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
  if (ret < 0) { // any error
    log_printf("[E] format error parsing %s", fpath);
    return 0;
  }

  (void)fclose(fd);

  // just add this to the list
  if (ctx.name != NULL && ctx.exec != NULL) {
    struct session* this_session = malloc(sizeof(struct session));
    if (this_session == NULL) return 0;

    *this_session = (struct session){
        .name = ctx.name,
        .exec = ctx.exec,
        .tryexec = ctx.tryexec,
        .type = session_type,
    };

    vec_push(cb_sessions, this_session);
  }

  return 0;
}

// This code is designed to be run purely single threaded
#define LIKELY_BOUND_SESSIONS 8
struct Vector get_avaliable_sessions() {
  struct Vector sessions = vec_new();
  vec_reserve(&sessions, LIKELY_BOUND_SESSIONS);

  cb_sessions = &sessions;
  for (size_t i = 0; i < (sizeof(sources) / sizeof(sources[0])); i++) {
    log_printf("[I] parsing into %s\n", sources[i].dir);
    session_type = sources[i].type;
    ftw(sources[i].dir, &fn, 1);
  }
  cb_sessions = NULL;

  return sessions;
}
