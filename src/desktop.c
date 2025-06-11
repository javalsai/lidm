// NOLINTBEGIN(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling,readability-function-cognitive-complexity)
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "desktop.h"
#include "macros.h"

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

    if (read_size <= 1) continue;

    if (buf[0] == '[' && buf[read_size - 2] == ']') {
      if (table_name != NULL) free(table_name);
      table_name = realloc(buf, read_size);
      table_name[read_size - 1] = '\0'; // newline
      buf = NULL;
      alloc_size = 0;
    } else {
      // Find '='
      size_t eq_idx = 0;
      while (buf[eq_idx] != '\0') {
        if (buf[eq_idx] == '=') break;
        eq_idx++;
      }
      // impossible with a min len of 1 (empty line)
      if (eq_idx == 0) continue;
      // Check its not end
      if (buf[eq_idx] != '=') {
        ret = -1;
        break;
      }

      // Key & Value
      char* key = buf;
      buf[eq_idx] = '\0'; // the equal
      char* value = &buf[eq_idx + 1];
      buf[read_size - 1] = '\0'; // the newline

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
