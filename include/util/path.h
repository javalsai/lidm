#ifndef UTIL_PATH_H
#define UTIL_PATH_H

#include "macros.h"

char* NULLABLE search_path(const char* NNULLABLE for_binary);
int execvpe(const char* NNULLABLE file, char* NULLABLE const argv[NNULLABLE],
            char* NULLABLE const envp[NNULLABLE]);

#endif /* UTIL_PATH_H */
