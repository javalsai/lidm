#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "keys.h"
#include "macros.h"
#include "ui.h"

static int selret_magic();

int find_keyname(enum Keys* at, const char* name) {
  for (size_t i = 0; i < LEN(KEY_MAPPINGS); i++) {
    if (strcmp(KEY_NAMES[i], name) == 0) {
      *at = (enum Keys)i;
      return 0;
    }
  }

  return -1;
}

struct option_keys find_ansi(const char* seq) {
  for (size_t i = 0; i < LEN(KEY_MAPPINGS); i++) {
    struct key_mapping mapping = KEY_MAPPINGS[i];
    for (size_t j = 0; mapping.sequences[j] != NULL; j++) {
      if (strcmp(mapping.sequences[j], seq) == 0) {
        return (struct option_keys){
            .is_some = true,
            .key = (enum Keys)i,
        };
      }
    }
  }
  return (struct option_keys){.is_some = false};
}

void read_press(uint8_t* length, char* out) {
  *length = 0;

  while (true) {
    if (read(STDIN_FILENO, &out[(*length)++], 1) != 1) {
      print_errno("read error");
      sleep(3);
      exit(1);
    }
    int selret = selret_magic();
    if (selret == -1) {
      print_errno("selret error");
    } else if (selret != 1) {
      out[*length] = '\0';
      return;
    }
  }
}

bool read_press_nb(uint8_t* length, char* out, struct timeval* tv) {
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  errno = 0;
  int ret = select(STDIN_FILENO + 1, &fds, NULL, NULL, tv);
  if (errno || ret <= 0) return false;

  read_press(length, out);
  return true;
}

// https://stackoverflow.com/a/48040042
static int selret_magic() {
  fd_set set;
  struct timeval timeout;
  FD_ZERO(&set);
  FD_SET(STDIN_FILENO, &set);
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  return select(1, &set, NULL, NULL, &timeout);
}
