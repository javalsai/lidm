#include <stdlib.h>

#include "util/vec.h"

const struct Vector VEC_NEW = {
    .length = 0,
    .capacity = 0,
    .pages = NULL,
};

struct Vector vec_from_raw(void** raw) {
  size_t len = 0;
  while (raw[len])
    len++;

  return (struct Vector){
      .length = len,
      .capacity = len,
      .pages = raw,
  };
}

void** vec_as_raw(struct Vector self) {
  if (vec_push(&self, NULL) != 0) return NULL;
  return self.pages;
}

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
