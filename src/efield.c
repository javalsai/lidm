#include <string.h>

#include "efield.h"
#include "ui.h"
#include "util.h"

struct editable_field efield_new(char* content) {
  struct editable_field efield;
  if (content != NULL) {
    efield.pos = strlen(content);
    memcpy(efield.content, content, strlen(content) + 1);
  } else {
    efield_trim(&efield, 0);
  }

  return efield;
}

void efield_trim(struct editable_field* self, u_char pos) {
  self->pos = pos;
  self->content[pos + 1] = '\0';
}

#define BACKSPACE_CODE 127
void efield_update(struct editable_field* self, char* update) {
  u_char insert_len = strlen(update);
  if (insert_len == 0) return;

  if (self->pos > strlen(self->content))
    self->pos = strlen(self->content); // WTF tho

  if (insert_len == 1) {
    // backspace
    if (*update == BACKSPACE_CODE) {
      if (self->pos == 0) return;
      char* curr = &self->content[self->pos];
      char* prev = (char*)utf8back(curr);
      memmove(prev, curr, strlen(self->content) - self->pos + 1);

      self->pos -= curr - prev;
      return;
    }
    // TODO: Del
  }

  // append
  if (strlen(update) + self->pos + 1 >= 255) {
    print_err("field too long");
  }

  // move the after pos, including nullbyte
  memmove(&self->content[self->pos + insert_len], &self->content[self->pos],
          strlen(self->content) - self->pos + 1);
  memcpy(&self->content[self->pos], update, insert_len);

  self->pos += insert_len;
}

// returns bool depending if it was able to "use" the seek
bool efield_seek(struct editable_field* self, char seek) {
  if (*self->content == '\0') return false;
  if (seek == 0) return false;

  u_char count = seek < 0 ? -seek : seek;
  char* ptr = &self->content[self->pos];
  char* start = ptr;

  while (count-- > 0) {
    if (seek < 0) {
      if (ptr == self->content) break;
      ptr = (char*)utf8back(ptr);
    } else {
      if (*ptr == '\0') break;
      ptr = (char*)utf8seek(ptr);
    }
  }

  self->pos = (u_char)(ptr - self->content);
  return ptr != start;
}
