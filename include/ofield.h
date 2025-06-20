#ifndef OFIELD_
#define OFIELD_

#include <stddef.h>
#include "efield.h"

// related vector is external, indexing at 1, 0 means empty and hence points to
// the editable_field
struct opts_field {
  size_t opts;
  size_t current_opt;
  struct editable_field efield;
};

struct opts_field ofield_new(size_t opts);
void ofield_toedit(struct opts_field* self, char* init);
void ofield_kbd_type(struct opts_field* self, char* typed, char* empty_default);
bool ofield_opts_seek(struct opts_field* self, char seek);
bool ofield_seek(struct opts_field* self, char seek);

u_char ofield_display_cursor_col(struct opts_field* self);

#endif
