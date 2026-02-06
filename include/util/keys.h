#ifndef UTIL_KEYS_H
#define UTIL_KEYS_H

#include <stdbool.h>
#include <sys/types.h>

#include "keys.h"

int find_keyname(enum Keys* at, const char* name);
struct option_keys {
  bool is_some;
  enum Keys key;
};
struct option_keys find_ansi(const char* seq);
void read_press(u_char* length, char* out);
// non blocking, waits up to tv or interrupt, returns true if actually read
bool read_press_nb(u_char* length, char* out, struct timeval* tv);

#endif /* UTIL_KEYS_H */
