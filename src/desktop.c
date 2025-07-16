// NOLINTBEGIN(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling,readability-function-cognitive-complexity)
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "desktop.h"

char* trim_str(char* str) {
  while (*str == ' ' || *str == '\t')
    str++;

  size_t i = strlen(str); // NOLINT(readability-identifier-length)
  while (i > 0) {
    if (str[i - 1] != ' ' && str[i - 1] != '\t' && str[i - 1] != '\n') break;
    i--;
  }
  str[i] = '\0';

  return str;
}

int read_desktop(FILE* fd, void* ctx,
                 struct status (*cb)(void* ctx, char* table, char* key,
                                     char* value)) {
  char* table_name = NULL;

  bool ret = -1;
  char* buf = NULL;
  size_t alloc_size = 0;
  size_t read_size;
  while ((read_size = getline(&buf, &alloc_size, fd)) > 0) {
    ret = 0;

    char* buf_start = trim_str(buf);
    size_t indent_size = buf_start - buf;

    if (read_size - indent_size < 1) continue;
    if (*buf_start == '#') continue;

    if (*buf_start == '[' && buf_start[strlen(buf_start) - 1] == ']') {
      if (table_name != NULL) free(table_name);
      buf_start[strlen(buf_start) - 1] = '\0';
      table_name = strdup(buf_start + 1);
      if (table_name == NULL) {
        ret = -1;
        break;
      }
    } else {
      // Find '='
      size_t eq_idx = 0;
      while (buf_start[eq_idx] != '\0') {
        if (buf_start[eq_idx] == '=') break;
        eq_idx++;
      }
      // impossible with a min len of 1 (empty line)
      if (eq_idx == 0) continue;
      // Check its not end
      if (buf_start[eq_idx] != '=') {
        ret = -1;
        break;
      }

      // Key & Value
      char* key = buf_start;
      buf_start[eq_idx] = '\0'; // the equal
      key = trim_str(key);
      char* value = &buf_start[eq_idx + 1];
      if (buf_start[read_size - 1] == '\n') buf_start[read_size - 1] = '\0';
      value = trim_str(value);

      // Callback
      struct status cb_ret = cb(ctx, table_name, key, value);
      if (cb_ret.finish) {
        ret = cb_ret.ret;
        break;
      }
    }
  }

  if (table_name != NULL) free(table_name);
  if (buf != NULL) free(buf);
  return ret;
}
// NOLINTEND(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling,readability-function-cognitive-complexity)
