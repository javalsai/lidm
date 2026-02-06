#include "util/utf8.h"

#define UTF8_CONT_MSK 0b11000000
#define UTF8_CONT_VAL 0b10000000
bool utf8_iscont(char byte) {
  return (byte & UTF8_CONT_MSK) == UTF8_CONT_VAL;
}

size_t utf8len(const char* str) {
  size_t len = 0;
  while (*str != '\0') {
    if (!utf8_iscont(*(str++))) len++;
  }

  return len;
}

size_t utf8len_until(const char* str, const char* until) {
  size_t len = 0;
  while (str < until) {
    if (!utf8_iscont(*(str++))) len++;
  }

  return len;
}

size_t utf8trunc(char* str, size_t n) {
  size_t bytes = 0;
  while (true) {
    if (str[bytes] == '\0') break;
    if (utf8_iscont(str[bytes])) {
      bytes++;
      continue;
    }
    if (n == 0) {
      str[bytes] = '\0';
      break;
    }
    bytes++;
    n--;
  }
  return bytes;
}

const char* utf8back(const char* str) {
  while (utf8_iscont(*(--str))) {
  }
  return str;
}
const char* utf8seek(const char* str) {
  while (utf8_iscont(*(++str))) {
  }
  return str;
}

const char* utf8seekn(const char* str, size_t n) {
  while (n > 0 && *str != '\0') {
    str = utf8seek(str);
    n--;
  }
  return str;
}
