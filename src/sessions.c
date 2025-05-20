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
  char *dir;
};
static const struct source_dir sources[] = {
    {XORG, "/usr/share/xsessions"},
    {WAYLAND, "/usr/share/wayland-sessions"},
};

static struct session __new_session(enum session_type type, char *name,
                                    const char *exec, const char *tryexec) {
  struct session __session;
  __session.type = type;
  strcln(&__session.name, name);
  strcln(&__session.exec, exec);
  strcln(&__session.tryexec, tryexec);

  return __session;
}

static struct Vector *cb_sessions = NULL;

// NOTE: commented printf's here would be nice to have debug logs if I ever
// implement it
static enum session_type session_type;
static int fn(const char *fpath, const struct stat *sb, int typeflag) {
  if (sb == NULL || !S_ISREG(sb->st_mode))
    return 0;

  /*printf("gonna open %s\n", fpath);*/
  FILE *fd = fopen(fpath, "r");
  if (fd == NULL) {
    perror("fopen");
    fprintf(stderr, "error opening file (r) '%s'\n", fpath);
    return 0;
  }

  u_char found = 0;
  size_t alloc_size = sb->st_blksize;

  char *name_buf = NULL;
  char *exec_buf = NULL;
  char *tryexec_buf = NULL;
  // This should be made a specific function
  while (true) {
    char *buf = malloc(sb->st_blksize);
    ssize_t read_size = getline(&buf, &alloc_size, fd);
    if (read_size == -1) {
      free(buf);
      break;
    }

    uint read;
    char *key = malloc(read_size + sizeof(char));
    char *value = malloc(read_size + sizeof(char));
    if ((read = sscanf(buf, "%[^=]=%[^\n]\n", key, value)) != 0) {
      if (strcmp(key, "Name") == 0) {
        found &= 0b001;
        name_buf = realloc(value, strlen(value) + sizeof(char));
      } else if (strcmp(key, "Exec") == 0) {
        found &= 0b010;
        exec_buf = realloc(value, strlen(value) + sizeof(char));
      } else if (strcmp(key, "TryExec") == 0) {
        found &= 0b100;
        tryexec_buf = realloc(value, strlen(value) + sizeof(char));
      } else {
        free(value);
      }
	}
    free(key);
	  free(buf);
    if (found == 0b111) break;
  }
  /*printf("\nend parsing...\n");*/

  fclose(fd);

  // just add this to the list
  if (name_buf != NULL && exec_buf != NULL) {
    struct session *session_i = malloc(sizeof (struct session));
    *session_i = __new_session(session_type, name_buf, exec_buf,
                               tryexec_buf == NULL ? "" : tryexec_buf);
    vec_push(cb_sessions, session_i);
  }

  if (name_buf != NULL)
    free(name_buf);
  if (exec_buf != NULL)
    free(exec_buf);
  if (tryexec_buf != NULL)
    free(tryexec_buf);

  return 0;
}

// This code is designed to be run purely single threaded
struct Vector get_avaliable_sessions() {
  struct Vector sessions = vec_new();

  cb_sessions = &sessions;
  for (size_t i = 0; i < (sizeof(sources) / sizeof(sources[0])); i++) {
    /*printf("recurring into %s\n", sources[i].dir);*/
    session_type = sources[i].type;
    ftw(sources[i].dir, &fn, 1);
  }
  cb_sessions = NULL;

  return sessions;
}
