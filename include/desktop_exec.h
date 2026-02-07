// TODO: rewrite properly
// NOLINTBEGIN(clang-diagnostic-nullability-completeness)

#ifndef DESKTOP_EXEC_H_
  #define DESKTOP_EXEC_H_

  #include "macros.h"

char* NULLABLE desktop_as_cmdline(char** args);
int parse_exec_string(const char* exec_s, int* arg_count, char*** args);
void free_parsed_args(int arg_count, char** args);

#endif
// NOLINTEND(clang-diagnostic-nullability-completeness)
