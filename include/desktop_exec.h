#ifndef DESKTOP_EXEC_H_
#define DESKTOP_EXEC_H_

int parse_exec_string(const char* exec_s, int* arg_count, char*** args);
void free_parsed_args(int arg_count, char** args);

#endif
