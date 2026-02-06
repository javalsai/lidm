#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "util/path.h"

// returns NULL on any error
// otherwise it returns the absolute path of the program that MUST BE FREED
//
// Should be almost completely posix compliant, except it won't resolve empty
// PATH entries relative to the cwd
char* NULLABLE search_path(const char* NNULLABLE for_binary) {
  if (strchr(for_binary, '/') != NULL) {
    // skip absolute paths
    return strdup(for_binary);
  }
  char* path_env = getenv("PATH");
  if (!path_env) return NULL;
  char* path = strdup(path_env);
  if (!path) return NULL;

  char* tok = strtok(path, ":");
  while (tok) {
    char* bin_path;
    asprintf(&bin_path, "%s/%s", tok, for_binary);
    if (!bin_path) {
      free(path);
      return NULL;
    }

    struct stat stat_buf;
    if (stat(bin_path, &stat_buf) == 0 && access(bin_path, X_OK) == 0) {
      free(path);
      return bin_path;
    }

    free(bin_path);
    tok = strtok(NULL, ":");
  }

  free(path);
  return NULL;
}

// This is present in glibc ONLY with GNU extensions, this aims to provide a
// musl compatible variant.
//
// Respects errno of exec functions family.
int execvpe(const char* NNULLABLE file, char* const argv[],
            char* const envp[]) {
  char* path = search_path(file);
  if (!path) {
    errno = ENOENT;
    return -1;
  }
  int ret = execve(path, argv, envp);
  free(path);
  return ret;
}
