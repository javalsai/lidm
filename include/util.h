#ifndef _UTILH_
#define _UTILH_

#include <keys.h>
#include <stdbool.h>
#include <sys/types.h>

enum keys find_ansi(char*);
void read_press(u_char*, char*);
void strcln(char **dest, const char *source);

#endif
