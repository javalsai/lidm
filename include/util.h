#ifndef UTILH_
#define UTILH_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#include "keys.h"

int find_keyname(enum keys* at, const char* name);
enum keys find_ansi(const char*);
void read_press(u_char*, char*);

bool utf8_iscont(char byte);
size_t utf8len(const char* str);
size_t utf8len_until(const char* str, const char* until);
const char* utf8back(const char* str);
const char* utf8seek(const char* str);
const char* utf8seekn(const char* str, size_t n);

struct Vector {
  uint32_t length;
  uint32_t capacity;
  void** pages;
};

extern const struct Vector VEC_NEW;
int vec_resize(struct Vector*, size_t size);
int vec_reserve(struct Vector*, size_t size);
int vec_reserve_exact(struct Vector*, size_t size);
int vec_push(struct Vector*, void* item);
void vec_free(struct Vector*);
void vec_clear(struct Vector*);
void vec_reset(struct Vector*);
void* vec_pop(struct Vector*); // won't free it, nor shrink vec list space
void* vec_get(struct Vector*, size_t index);

#endif
