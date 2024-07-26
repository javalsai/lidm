#include <ftw.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <sessions.h>
#include <util.h>

struct source_dir {
  enum session_type type;
  char *dir;
};
static const struct source_dir sources[] = {
    {XORG, "/usr/share/xsessions"},
    {WAYLAND, "/usr/share/wayland-sessions"},
};
static const size_t sources_size = sizeof(sources) / sizeof(sources[0]);

static struct session __new_session(enum session_type type, char *name,
                                    const char *exec, const char *tryexec) {
  struct session __session;
  __session.type = type;
  strcln(&__session.name, name);
  strcln(&__session.exec, exec);
  strcln(&__session.tryexec, tryexec);

  return __session;
}

static const u_int8_t bs = 16;
static const u_int8_t unit_size = sizeof(struct session);

static u_int16_t alloc_size = bs;
static u_int16_t used_size = 0;

static struct session *sessions = NULL;
static struct sessions_list *__sessions_list = NULL;

// NOTE: commented printf's here would be nice to have debug logs if I ever implement it
static enum session_type session_type;
static int fn(const char *fpath, const struct stat *sb, int typeflag) {
  // practically impossible to reach this
  // but will prevent break
  if (used_size == 0xffff)
    return 0;

  if (sb == NULL || !S_ISREG(sb->st_mode))
    return 0;

  /*printf("gonna open %s\n", fpath);*/
  FILE *fd = fopen(fpath, "r");
  if (fd == NULL) {
    perror("fopen");
    fprintf(stderr, "error opening file (r) %s\n", fpath);
    return 0;
  }

  u_char found = 0;
  size_t alloc_size = sb->st_blksize;

  char *name_buf = NULL;
  char *exec_buf = NULL;
  char *tryexec_buf = NULL;
  while (true) {
    /*printf(".");*/
    char *buf = malloc(sb->st_blksize);
    ssize_t read_size = getline(&buf, &alloc_size, fd);
    if (read_size == -1) {
      free(buf);
      break;
    }

    uint read;
    char *key = malloc(read_size);
    if ((read = sscanf(buf, "%[^=]=%[^\n]\n", key, buf)) != 0) {
      if (strcmp(key, "Name") == 0) {
        found &= 0b001;
        name_buf = realloc(buf, read);
      } else if (strcmp(key, "Exec") == 0) {
        found &= 0b010;
        exec_buf = realloc(buf, read);
      } else if (strcmp(key, "TryExec") == 0) {
        found &= 0b100;
        tryexec_buf = realloc(buf, read);
      } else
        free(buf);
    } else
      free(buf);
    free(key);

    if (found == 0b111)
      break;
  }
  /*printf("\nend parsing...\n");*/

  fclose(fd);

  // just add this to the list
  if (name_buf != NULL && exec_buf != NULL) {
    /*printf("gonna add to session list\n");*/
    if (used_size >= alloc_size) {
      alloc_size += bs;
      sessions = realloc(sessions, alloc_size * unit_size);
    }

    /*printf("n %s\ne %s\nte %s\n", name_buf, exec_buf, tryexec_buf);*/
    sessions[used_size] = __new_session(session_type, name_buf, exec_buf,
                                        tryexec_buf == NULL ? "" : tryexec_buf);

    used_size++;
  }

  if (name_buf != NULL)
    free(name_buf);
  if (exec_buf != NULL)
    free(exec_buf);
  if (tryexec_buf != NULL)
    free(tryexec_buf);

  return 0;
}

static struct sessions_list __list;
// This code is designed to be run purely single threaded
struct sessions_list *get_avaliable_sessions() {
  if (sessions != NULL)
    return __sessions_list;
  else
    sessions = malloc(alloc_size * unit_size);

  for (uint i = 0; i < sources_size; i++) {
    session_type = sources[i].type;
    /*printf("recurring into %s\n", sources[i].dir);*/
    ftw(sources[i].dir, &fn, 1);
  }

  sessions = realloc(sessions, used_size * unit_size);

  __list.length = used_size;
  __list.sessions = sessions;
  return __sessions_list = &__list;
}
