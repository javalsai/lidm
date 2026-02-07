#ifndef UTIL_UTF8_H
#define UTIL_UTF8_H

#include <stdbool.h>
#include <stddef.h>

bool utf8_iscont(char byte);
size_t utf8len(const char* str);
size_t utf8len_until(const char* str, const char* until);
size_t utf8trunc(char* str, size_t n);
const char* utf8back(const char* str);
const char* utf8seek(const char* str);
const char* utf8seekn(const char* str, size_t n);

#endif /* UTIL_UTF8_H */
