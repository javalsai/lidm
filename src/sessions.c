#include <ftw.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

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

static struct session new_session(enum session_type type,
                                    char* name,
                                    const char* exec,
                                    const char* tryexec) {
  struct session session;
  session.type = type;
  strcln(&session.name, name);
  strcln(&session.exec, exec);
  strcln(&session.tryexec, tryexec);

  return session;
}

static struct Vector* cb_sessions = NULL;

// NOTE: commented printf's here would be nice to have debug logs if I ever
// implement it
#define LN_NAME 0b0001
#define LN_EXEC 0b0010
#define LN_TEXEC 0b0100
#define LN_ALL (LN_NAME | LN_EXEC | LN_TEXEC)
static enum session_type session_type;
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
static int fn(const char* fpath, const struct stat* sb, int typeflag) {
  if (!S_ISREG(sb->st_mode)) return 0;

  // printf("gonna open %s\n", fpath);
  FILE* fd = fopen(fpath, "r");
  if (fd == NULL) {
    perror("fopen");
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
    (void)fprintf(stderr, "error opening file (r) '%s'\n", fpath);
    return 0;
  }

  u_char found = 0;

  char* name_buf = NULL;
  char* exec_buf = NULL;
  char* tryexec_buf = NULL;
  // This should be made a specific function
  // Emm, if anything goes wrong just free the inner loop and `break;` fd and
  // the rest is handled after
  char* buf = NULL;
  size_t alloc_size = 0;
  size_t read_size;
  while ((read_size = getline(&buf, &alloc_size, fd)) != -1) {
    char* key = malloc(read_size + sizeof(char));
    if (key == NULL) {
      free(buf);
      break;
    }
    char* value = malloc(read_size + sizeof(char));
    if (value == NULL) {
      free(buf);
      free(key);
      break;
    }
    value[0] = '\0'; // I'm not sure if sscanf would null this string out
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
    if (sscanf(buf, "%[^=]=%[^\n]\n", key, value) == 2) {
      if (strcmp(key, "Name") == 0) {
        found &= LN_EXEC;
        if(name_buf != NULL) free(name_buf);
        name_buf = realloc(value, strlen(value) + sizeof(char));
      } else if (strcmp(key, "Exec") == 0) {
        found &= LN_EXEC;
        if(exec_buf != NULL) free(exec_buf);
        exec_buf = realloc(value, strlen(value) + sizeof(char));
      } else if (strcmp(key, "TryExec") == 0) {
        found &= LN_TEXEC;
        if(tryexec_buf != NULL) free(tryexec_buf);
        tryexec_buf = realloc(value, strlen(value) + sizeof(char));
      } else {
        free(value);
      }
    } else {
      free(value);
    }
    free(key);
    // if (found == LN_ALL) break;
  }

  if(buf != NULL) free(buf);
  (void)fclose(fd);
  // printf("\nend parsing...\n");

  // just add this to the list
  if (name_buf != NULL && exec_buf != NULL) {
    struct session* session_i = malloc(sizeof(struct session));
    *session_i = new_session(session_type, name_buf, exec_buf,
                               tryexec_buf == NULL ? "" : tryexec_buf);
    vec_push(cb_sessions, session_i);
  }

  if (name_buf != NULL) free(name_buf);
  if (exec_buf != NULL) free(exec_buf);
  if (tryexec_buf != NULL) free(tryexec_buf);

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
