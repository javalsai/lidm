#ifndef EFIELDH_
#define EFIELDH_

#include <stdbool.h>
#include <sys/types.h>

struct editable_field {
  u_char length;
  u_char pos;
  char content[255];
};

struct editable_field field_new(char* content);
void field_trim(struct editable_field* self, u_char pos);
void field_update(struct editable_field* self, char* update);
bool field_seek(struct editable_field* self, char seek);

#endif
