#include <stddef.h>

#include "config.h"
#include "efield.h"
#include "ofield.h"
#include "ui.h"
#include "util.h"

struct opts_field ofield_new(size_t opts) {
  if (opts == 0) {
    return (struct opts_field){
        .opts = 0,
        .current_opt = 0,
        .efield = efield_new(""),
    };
  }

  return (struct opts_field){
      .opts = opts,
      .current_opt = 1,
  };
}

void ofield_toedit(struct opts_field* self, char* init) {
  self->current_opt = 0;
  self->efield = efield_new(init);
}

void ofield_kbd_type(struct opts_field* self, char* typed,
                     char* empty_default) {
  if (self->current_opt != 0) ofield_toedit(self, empty_default);
  efield_update(&self->efield, typed);
}

bool ofield_opts_seek(struct opts_field* self, char seek) {
  // no options or (a single option but its selected instead of on edit)
  if (self->opts == 0 || (self->opts == 1 && self->current_opt != 0))
    return false;

  self->current_opt =
      1 + ((self->current_opt - 1 + seek + self->opts) % self->opts);
  ui_update_ofield(self);
  return true;
}

bool ofield_seek(struct opts_field* self, char seek) {
  if (self->current_opt == 0) {
    if (efield_seek(&self->efield, seek)) {
      return true;
    }
  }

  if (self->opts == 0) return false;
  ofield_opts_seek(self, seek);
  return true;
}

u_char ofield_display_cursor_col(struct opts_field* self, u_char maxlen) {
  if (self->current_opt == 0) {
    u_char display_len = utf8len(self->efield.content);
    u_char pos = utf8len_until(self->efield.content,
                               &self->efield.content[self->efield.pos]);

    if (display_len > maxlen) {
      if (pos < maxlen / 2) {
        return pos;
      }
      if (display_len - pos < maxlen / 2) {
        return maxlen - (display_len - pos);
      }
      return maxlen / 2;
    }

    return pos;
  }
  return 0;
}
