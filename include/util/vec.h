#ifndef UTIL_VEC_H
#define UTIL_VEC_H

#include <stddef.h>
#include <stdint.h>

struct Vector {
  uint32_t length;
  uint32_t capacity;
  void** pages;
};

struct Vector vec_from_raw(void** raw);
void** vec_as_raw(struct Vector self);
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

#endif /* UTIL_VEC_H */
