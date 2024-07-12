#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <keys.h>
#include <ui.h>
#include <util.h>

static int selret_magic();

void strcln(char **dest, const char *source) {
  *dest = malloc(strlen(source) + sizeof(char));
  strcpy(*dest, source);
}

enum keys find_ansi(char *seq) {
  for (size_t i = 0; i < sizeof(key_mappings) / sizeof(key_mappings[0]); i++) {
    struct key_mapping mapping = key_mappings[i];
    for (size_t j = 0; mapping.sequences[j] != NULL; j++) {
      if (strcmp(mapping.sequences[j], seq) == 0) {
        return (enum keys)i;
      }
    }
  }
  return -1;
}

// https://stackoverflow.com/a/48040042
void read_press(u_char *length, char *out) {
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

static int selret_magic() {
  fd_set set;
  struct timeval timeout;
  FD_ZERO(&set);
  FD_SET(STDIN_FILENO, &set);
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  return select(1, &set, NULL, NULL, &timeout);
}
