#include <string.h>

#include <efield.h>
#include <ui.h>

struct editable_field field_new(char *content) {
  struct editable_field __efield;
  if (content != NULL) {
    __efield.length = __efield.pos = strlen(content);
    memcpy(__efield.content, content, __efield.length);
  } else {
    field_trim(&__efield, 0);
  }
  __efield.content[__efield.length] = '\0';
  return __efield;
}

void field_trim(struct editable_field *field, u_char pos) {
  field->length = field->pos = pos;
  field->content[field->length] = '\0';
}

void field_update(struct editable_field *field, char *update) {
  u_char insert_len = strlen(update);
  if (insert_len == 0)
    return;

  if (field->pos > field->length)
    field->pos = field->length; // WTF
  if (insert_len == 1) {
    // backspace
    if (*update == 127) {
      if (field->pos == 0)
        return;
      if (field->pos < field->length) {
        memmove(&field->content[field->pos - 1], &field->content[field->pos],
                field->length - field->pos);
      }
      (field->pos)--;
      (field->length)--;
      field->content[field->length] = '\0';
      return;
    }
  }

  // append
  if (field->length + field->pos >= 255) {
    print_err("field too long");
  }
  if (field->pos < field->length) {
    // move with immediate buffer
    memmove(&field->content[field->pos + insert_len],
            &field->content[field->pos], field->length - field->pos);
  }
  memcpy(&field->content[field->pos], update, insert_len);

  field->pos += insert_len;
  field->length += insert_len;
  field->content[field->length] = '\0';
}

// returns bool depending if it was able to "use" the seek
bool field_seek(struct editable_field *field, char seek) {
  if (field->length == 0)
    return false;

  if (seek < 0 && -seek > field->pos)
    field->pos = 0;
  else if (seek > 0 && 255 - field->pos < seek)
    field->pos = 255;
  else
    field->pos += seek;

  if (field->pos > field->length)
    field->pos = field->length;

  return true;
}
