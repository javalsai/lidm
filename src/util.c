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

int find_keyname(enum keys* at, const char* name) {
  for (size_t i = 0; i < LEN(KEY_MAPPINGS); i++) {
    if (strcmp(KEY_NAMES[i], name) == 0) {
      *at = (enum keys)i;
      return 0;
    }
  }

  return -1;
}

enum keys find_ansi(const char* seq) {
  for (size_t i = 0; i < LEN(KEY_MAPPINGS); i++) {
    struct key_mapping mapping = KEY_MAPPINGS[i];
    for (size_t j = 0; mapping.sequences[j] != NULL; j++) {
      if (strcmp(mapping.sequences[j], seq) == 0) {
        return (enum keys)i;
      }
    }
  }
  return -1;
}

void read_press(u_char* length, char* out) {
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

// UTF-8 shii
#define UTF8_CONT_MSK 0b11000000
#define UTF8_CONT_VAL 0b10000000
bool utf8_iscont(char byte) {
  return (byte & UTF8_CONT_MSK) == UTF8_CONT_VAL;
}

size_t utf8len(const char* str) {
  size_t len = 0;
  while (*str != '\0') {
    if (!utf8_iscont(*(str++))) len++;
  }

  return len;
}

size_t utf8len_until(const char* str, const char* until) {
  size_t len = 0;
  while (str < until) {
    if (!utf8_iscont(*(str++))) len++;
  }

  return len;
}

const char* utf8back(const char* str) {
  while (utf8_iscont(*(--str))) {
  }
  return str;
}
const char* utf8seek(const char* str) {
  while (utf8_iscont(*(++str))) {
  }
  return str;
}

const char* utf8seekn(const char* str, size_t n) {
  while (n > 0 && *str != '\0') {
    str = utf8seek(str);
    n--;
  }
  return str;
}

// Vector shii
const struct Vector VEC_NEW = {
    .length = 0,
    .capacity = 0,
    .pages = NULL,
};

int vec_resize(struct Vector* self, size_t size) {
  void** new_location =
      (void**)realloc((void*)self->pages, size * sizeof(void*));
  if (new_location != NULL) {
    if (self->length > size) self->length = size;
    self->capacity = size;
    self->pages = new_location;
  } else {
    return -1;
  }
  return 0;
}

int vec_reserve(struct Vector* self, size_t size) {
  uint32_t new_capacity = self->capacity;
  while (self->length + size > new_capacity) {
    new_capacity = new_capacity + (new_capacity >> 1) +
                   1; // cap * 1.5 + 1; 0 1 2 4 7 11...
  }
  return vec_resize(self, new_capacity);
}

int vec_reserve_exact(struct Vector* self, size_t size) {
  uint32_t needed_capacity = self->length + size;
  if (self->capacity < needed_capacity) {
    return vec_resize(self, needed_capacity);
  }
  return 0;
}

int vec_push(struct Vector* self, void* item) {
  int res_ret = vec_reserve(self, 1);
  if (res_ret != 0) return res_ret;

  self->pages[self->length++] = item;
  return 0;
}

void vec_free(struct Vector* self) {
  while (self->length > 0)
    free(self->pages[--self->length]);

  vec_clear(self);
}

void vec_clear(struct Vector* self) {
  free((void*)self->pages);
  vec_reset(self);
}

void vec_reset(struct Vector* self) {
  *self = (struct Vector){
      .length = 0,
      .capacity = 0,
      .pages = NULL,
  };
}

void* vec_pop(struct Vector* self) {
  if (self->length == 0) return NULL;

  return self->pages[--self->length];
}

void* vec_get(struct Vector* self, size_t index) {
  if (index >= self->length) return NULL;

  return self->pages[index];
}
