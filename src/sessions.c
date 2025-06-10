#include <ftw.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "desktop.h"
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

// static struct session new_session(enum session_type type, char* name,
//                                   const char* exec, const char* tryexec) {
//   struct session session;
//   session.type = type;
//   strcln(&session.name, name);
//   strcln(&session.exec, exec);
//   strcln(&session.tryexec, tryexec);

//   return session;
// }

static struct Vector* cb_sessions = NULL;

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
struct status cb(void* _ctx, char* NULLABLE table, char* key, char* value) {
  struct session* ctx = (struct session*)_ctx;
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
    *copy_at = malloc((strlen(value) + 1) * sizeof(char));
    if (*copy_at == NULL) {
      ret.finish = true;
      ret.ret = -1; // malloc error
    }

    strcpy(*copy_at, value);
  }

  if (ctx->name != NULL && ctx->exec != NULL && ctx->tryexec != NULL) {
    ret.finish = true;
    ret.ret = 0;
  }

  return ret;
}

static enum session_type session_type;
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
static int fn(const char* fpath, const struct stat* sb, int typeflag) {
  if (!S_ISREG(sb->st_mode)) return 0;

  struct session* ctx = malloc(sizeof(struct session));
  if (ctx == NULL) return 0;
  ctx->name = NULL;
  ctx->exec = NULL;
  ctx->tryexec = NULL;

  FILE* fd = fopen(fpath, "r");
  if (fd == NULL) {
    free(ctx);
    perror("fopen");
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
    (void)fprintf(stderr, "error opening file (r) '%s'\n", fpath);
    return 0;
  }

  int ret = read_desktop(fd, ctx, &cb);
  if (ret < 0) { // any error
    free(ctx);
    return 0;
  }

  (void)fclose(fd);

  // just add this to the list
  if (ctx->name != NULL && ctx->exec != NULL) {
    ctx->type = session_type;
    vec_push(cb_sessions, ctx);
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
    /*printf("recurring into %s\n", sources[i].dir);*/
    session_type = sources[i].type;
    ftw(sources[i].dir, &fn, 1);
  }
  cb_sessions = NULL;

  return sessions;
}
