#ifndef UTILH_
#define UTILH_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>

#include "keys.h"

int find_keyname(enum keys* at, const char* name);
enum keys find_ansi(const char* seq);
void read_press(u_char* length, char* out);
// non blocking, waits up to tv or interrupt, returns true if actually read
bool read_press_nb(u_char* length, char* out, struct timeval* tv);

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
int vec_resize(struct Vector* self, size_t size);
int vec_reserve(struct Vector* self, size_t size);
int vec_reserve_exact(struct Vector* self, size_t size);
int vec_push(struct Vector* self, void* item);
void vec_free(struct Vector* self);
void vec_clear(struct Vector* self);
void vec_reset(struct Vector* self);
void* vec_pop(struct Vector* self); // won't free it, nor shrink vec list space
void* vec_get(struct Vector* self, size_t index);

#endif
