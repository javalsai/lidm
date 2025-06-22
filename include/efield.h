#ifndef EFIELDH_
#define EFIELDH_

#include <stdbool.h>
#include <sys/types.h>

// holds also the max string buffer in itself, not dynamic
struct editable_field {
  u_char pos;
  char content[255];
};

struct editable_field efield_new(char* content);
void efield_trim(struct editable_field* self, u_char pos);
void efield_update(struct editable_field* self, char* update);
bool efield_seek(struct editable_field* self, char seek);

#endif
