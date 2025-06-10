#include <string.h>

#include "efield.h"
#include "ui.h"

// NOLINTBEGIN(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)

struct editable_field field_new(char* content) {
  struct editable_field efield;
  if (content != NULL) {
    efield.length = efield.pos = strlen(content);
    memcpy(efield.content, content, efield.length);
  } else {
    field_trim(&efield, 0);
  }
  efield.content[efield.length] = '\0';
  return efield;
}

void field_trim(struct editable_field* self, u_char pos) {
  self->length = self->pos = pos;
  self->content[self->length] = '\0';
}

// NOLINTNEXTLINE(modernize-macro-to-enum)
#define BACKSPACE_CODE 127
void field_update(struct editable_field* self, char* update) {
  u_char insert_len = strlen(update);
  if (insert_len == 0) return;

  if (self->pos > self->length) self->pos = self->length; // WTF
  if (insert_len == 1) {
    // backspace
    if (*update == BACKSPACE_CODE) {
      if (self->pos == 0) return;
      if (self->pos < self->length) {
        memmove(&self->content[self->pos - 1], &self->content[self->pos],
                self->length - self->pos);
      }
      (self->pos)--;
      (self->length)--;
      self->content[self->length] = '\0';
      return;
    }
  }

  // append
  if (self->length + self->pos >= 255) {
    print_err("field too long");
  }
  if (self->pos < self->length) {
    // move with immediate buffer
    memmove(&self->content[self->pos + insert_len],
            &self->content[self->pos], self->length - self->pos);
  }
  memcpy(&self->content[self->pos], update, insert_len);

  self->pos += insert_len;
  self->length += insert_len;
  self->content[self->length] = '\0';
}

// returns bool depending if it was able to "use" the seek
bool field_seek(struct editable_field* self, char seek) {
  if (self->length == 0) return false;

  if (seek < 0 && -seek > self->pos)
    self->pos = 0;
  else if (seek > 0 && 255 - self->pos < seek)
    self->pos = 255;
  else
    self->pos += seek;

  if (self->pos > self->length) self->pos = self->length;

  return true;
}

// NOLINTEND(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
