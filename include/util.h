#ifndef _UTILH_
#define _UTILH_

#include "keys.h"
#include "stdbool.h"
#include "stdint.h"
#include "sys/types.h"

enum keys find_keyname(char *);
enum keys find_ansi(char *);
void read_press(u_char *, char *);
void strcln(char **dest, const char *source);

struct Vector {
    uint32_t length;
    uint32_t alloc_len;
    uint16_t alloc_size;
    void** pages;
};

struct Vector vec_new();
int vec_push(struct Vector*, void* item);
void vec_free(struct Vector*);
void vec_clear(struct Vector*);
void vec_reset(struct Vector*);
void* vec_pop(struct Vector*); // won't free it, nor shrink vec list space
void* vec_get(struct Vector*, uint32_t index);

#endif
