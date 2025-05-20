#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "keys.h"
#include "ui.h"
#include "util.h"

static int selret_magic();

void strcln(char **dest, const char *source) {
  *dest = malloc(strlen(source) + sizeof(char));
  strcpy(*dest, source);
}

enum keys find_keyname(char *name) {
  for (size_t i = 0; i < sizeof(key_mappings) / sizeof(key_mappings[0]); i++) {
    if (strcmp(key_names[i], name) == 0)
      return (enum keys)i;
  }

  perror("key not found");
  exit(1);
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


// Vector shii
struct Vector vec_new() {
  struct Vector vec;
  vec_reset(&vec);
  return vec;
}

int vec_push(struct Vector* vec, void* item) {
  if (vec->length >= vec->alloc_len) {
    uint32_t new_size = vec->alloc_len + vec->alloc_size;
    void **new_location = realloc(vec->pages, vec->alloc_size);
    if (new_location != NULL) {
      vec->alloc_size = new_size;
      vec->pages = new_location;
    } else {
      return -1;
    }
  }

  vec->pages[vec->length] = item;
  vec->length++;
  return 0;
}

void vec_free(struct Vector* vec) {
  while(vec->length > 0)
    free(vec->pages[--vec->length]);

  vec_clear(vec);
}

void vec_clear(struct Vector* vec) {
  free(vec->pages);
  vec_reset(vec);
}

void vec_reset(struct Vector* vec) {
  vec->length = 0;
  vec->alloc_len = 0;
  vec->alloc_size = 4096; // 4KiB page size?
  vec->pages = NULL;
}

void* vec_pop(struct Vector* vec) {
  if (vec->length == 0)
    return NULL;

  return vec->pages[--vec->length];
}

void* vec_get(struct Vector* vec, size_t index) {
  if (index >= vec->length)
    return NULL;

  return vec->pages[index];
}
