#include <ftw.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <sessions.h>
#include <util.h>

static const char *sources[][2] = {
  { "xorg", "/usr/share/xsessions" },
  { "wl", "/usr/share/wayland-sessions" },
};
static const size_t sources_size = sizeof(sources) / sizeof(sources[0]);

static struct session __new_session(const char *type, char *name, const char *path) {
  struct session __session;
  __session.type = type;
  strcln(&__session.name, name);
  strcln(&__session.path, path);

  return __session;
}

static const u_int8_t bs = 16;
static const u_int8_t unit_size = sizeof(struct session);

static u_int16_t alloc_size = bs;
static u_int16_t used_size = 0;

static struct session *sessions = NULL;
static struct sessions_list *__sessions_list = NULL;

static const char* session_type;
static int fn(const char *fpath, const struct stat *sb, int typeflag) {
  // practically impossible to reach this
  // but will prevent break
  if (used_size == 0xffff)
    return 0;

  if (!S_ISREG(sb->st_mode))
    return 0;

  FILE *fd = fopen(fpath, "r");
  if (fd == NULL) {
    fprintf(stderr, "error opening file (r) %s", fpath);
    return 0;
  }

  bool found = false;
  size_t alloc_size = sb->st_blksize;
  char *buf = malloc(sb->st_blksize);
  while (true) {
    buf = realloc(buf, sb->st_blksize);
    ssize_t read_size = getline(&buf, &alloc_size, fd);
    if (read_size == -1)
      break;

    uint read;
    if ((read = sscanf(buf, "Name=%s\n", buf)) != 0) {
      found = true;
      buf = realloc(buf, read);
      break;
    }
  }

  // just add this to the list
  if (found) {
    if (used_size >= alloc_size) {
      alloc_size += bs;
      sessions = realloc(sessions, alloc_size * unit_size);
    }

    sessions[used_size] = __new_session(session_type, buf, fpath);
    used_size++;
  }

  free(buf);
  return 0;
}

static struct sessions_list __list;
// This code is designed to be run purely sinlge threaded
struct sessions_list *get_avaliable_sessions() {
  if (sessions != NULL)
    return __sessions_list;
  else
    sessions = malloc(alloc_size * unit_size);

  for (uint i = 0; i < sources_size; i++) {
    session_type = sources[i][0];
    ftw(sources[i][1], &fn, 1);
  }

  sessions = realloc(sessions, used_size * unit_size);

  __list.length = used_size;
  __list.sessions = sessions;
  return __sessions_list = &__list;
}
