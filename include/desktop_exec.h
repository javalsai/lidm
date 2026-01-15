// TODO: rewrite properly
// NOLINTBEGIN(clang-diagnostic-nullability-completeness)

#ifndef DESKTOP_EXEC_H_
  #define DESKTOP_EXEC_H_

  #include "macros.h"

int execvpe_desktop(char** args, char* NNULLABLE* NNULLABLE envlist);
int parse_exec_string(const char* exec_s, int* arg_count, char*** args);
void free_parsed_args(int arg_count, char** args);

#endif
// NOLINTEND(clang-diagnostic-nullability-completeness)
