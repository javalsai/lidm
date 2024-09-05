#ifndef _EFIELDH_
#define _EFIELDH_

#include <stdbool.h>
#include <sys/types.h>

struct editable_field {
  u_char length;
  u_char pos;
  char content[255];
};

struct editable_field field_new(char *);
void field_trim(struct editable_field *, u_char);
void field_update(struct editable_field *, char *);
bool field_seek(struct editable_field *, char);

#endif
