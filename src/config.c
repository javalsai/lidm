#include <errno.h>
#include <linux/fs.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "config.h"
#include "desktop.h"
#include "log.h"
#include "macros.h"
#include "util.h"

#define UPPER_HALF_BYTE 4
int parse_hex(char* _at, char x1, char x2) {
  // make linter happy
  u_char* at = (u_char*)_at;

  *at = 0;

  if ('0' <= x1 && x1 <= '9') {
    *at += (x1 - '0') << UPPER_HALF_BYTE;
  } else if ('A' <= x1 && x1 <= 'F') {
    *at += (x1 - 'A' + 10) << UPPER_HALF_BYTE;
  } else if ('a' <= x1 && x1 <= 'f') {
    *at += (x1 - 'a' + 10) << UPPER_HALF_BYTE;
  } else {
    return -1;
  }

  if ('0' <= x2 && x2 <= '9') {
    *at += (x2 - '0');
  } else if ('A' <= x2 && x2 <= 'F') {
    *at += (x2 - 'A' + 10);
  } else if ('a' <= x2 && x2 <= 'f') {
    *at += (x2 - 'a' + 10);
  } else {
    return -1;
  }

  return 0;
}

struct parser_error {
  const char* NULLABLE msg;
  size_t col;
};

#define FAIL(str)                \
  return (struct parser_error) { \
    str, p - raw                 \
  }
#define NOFAIL return (struct parser_error){NULL, 0}
struct parser_error parse_str_inplace(char* raw) {
  // reader pointer
  char* p = raw; // NOLINT(readability-identifier-length)
  if (*p != '"') FAIL("Strign not quoted");
  p++;

  // writer iterator, by nature will always be under the reader
  size_t i = 0; // NOLINT(readability-identifier-length)
  while (*p != '\0') {
    if (*p == '"') {
      if (p[1] == '\0') {
        raw[i] = '\0';
        NOFAIL;
      }
      FAIL("String finished but there's content after");
    }
    if (*p == '\\') {
      p++;
      if (*p == '\0') break;
      switch (*p) {
        case '\\':
          raw[i++] = '\\';
          break;
        case 't':
          raw[i++] = '\t';
          break;
        case 'n':
          raw[i++] = '\n';
          break;
        case '\"':
          raw[i++] = '\"';
          break;
        case '\'':
          raw[i++] = '\'';
          break;
        case 'x':
          if (p[1] == '\0' || p[2] == '\0') goto unfinished;
          if (parse_hex(&raw[i++], p[1], p[2]) != 0)
            FAIL("Invalid hex escape sequence");
          p += 2;
          break;
        default:
          FAIL("Invalid escape variant");
      }
    } else
      raw[i++] = *p;

    p++;
  }

unfinished:
  FAIL("Unfinished string");
}
#undef FAIL
#undef NOFAIL

#define FAIL(str) return (struct parser_error){str, 0}
#define NOFAIL return (struct parser_error){NULL, 0}
union typ_ptr {
  char** string;
  long long* number;
  bool* boolean;
  enum keys* key;
  struct Vector* vec;
  uintptr_t ptr;
};
struct parser_error parse_key(enum introspection_type typ, union typ_ptr at,
                              char* key, size_t offset) {
  char* aux_str = NULL;
  struct parser_error aux_err;

  switch (typ) {
    case STRING:
      aux_str = strdup(key);
      if (!aux_str) FAIL("allocation failure");
      aux_err = parse_str_inplace(aux_str);
      if (aux_err.msg) {
        free(aux_str);
        return aux_err;
      }
      // FIXME: it be broken, prob the ptr arithmetic or smth, we mem leak
      // instead 😎 If the ptr is not the default it means it was prev
      // allocated, we should free if (*(char**)((uintptr_t)(&default_config) +
      // offset) != *at.string) {
      //   free(*at.string);
      // }
      *at.string = aux_str;
      break;
    case BOOL:
      if (strcmp(key, "true") == 0)
        *at.boolean = true;
      else if (strcmp(key, "false") == 0)
        *at.boolean = false;
      else {
        FAIL("Invalid key value, wasn't 'true' nor 'false'");
      }
      break;
    case NUMBER:
      errno = 0;
      *at.number = strtol(key, NULL, 10);
      if (errno) {
        FAIL("strtol failed");
      }
      break;
    case KEY:
      // NOLINTNEXTLINE(performance-no-int-to-ptr)
      if (find_keyname(at.key, key) != 0) {
        FAIL("Keyboard KEY name not found");
      }
      break;
    case STRING_ARRAY:
      aux_str = strdup(key);
      if (!aux_str) FAIL("allocation failure");
      aux_err = parse_str_inplace(aux_str);
      if (aux_err.msg) {
        free(aux_str);
        return aux_err;
      }
      vec_push(at.vec, aux_str);
      break;
  }

  NOFAIL;
}
#undef FAIL
#undef NOFAIL

// NOLINTBEGIN(readability-identifier-length,readability-function-cognitive-complexity)
struct status config_line_handler(void* _config, char* table, char* k,
                                  char* v) {
  struct config* config = (struct config*)_config;
  struct status ret = {.finish = false};

  const struct introspection_table* this_intros_table = NULL;
  for (size_t i = 0; i < LEN(CONFIG_INSTROSPECTION); i++) {
    if (table == NULL) {
      if (table != CONFIG_INSTROSPECTION[i].tname) continue;
    } else if (strcmp(CONFIG_INSTROSPECTION[i].tname, table) != 0)
      continue;
    this_intros_table = &CONFIG_INSTROSPECTION[i];
    break;
  }
  if (this_intros_table == NULL) {
    log_printf("[E] table '%s' is not part of the config\n", table);
    return ret;
  }

  const struct introspection_item* this_intros_key = NULL;
  for (size_t i = 0; i < this_intros_table->len; i++) {
    if (strcmp(this_intros_table->intros[i].name, k) != 0) continue;
    this_intros_key = &this_intros_table->intros[i];
    break;
  }
  if (this_intros_key == NULL) {
    log_printf("[E] key '%s' is not part of the table '%s' in config\n", k,
               table);
    return ret;
  }

  size_t offset = this_intros_table->offset + this_intros_key->offset;
  union typ_ptr k_addr = {.ptr = (uintptr_t)config + offset};

  log_printf("[I] parsing [%s.%s] as %s\n", table, k,
             INTROS_TYS_NAMES[this_intros_key->typ]);
  struct parser_error err = parse_key(this_intros_key->typ, k_addr, v, offset);
  if (err.msg != NULL) {
    log_printf("[E] cfg parser, failed to parse [%s.%s] (%s): %s\n", table, k,
               INTROS_TYS_NAMES[this_intros_key->typ], err.msg);
    log_printf("%s\n%*c^\n", v, err.col);
    return ret;
  }

  if (this_intros_key->typ == NUMBER)
    log_printf("[I] cfg parsed [%s.%s] (%lld)\n", table, k, *k_addr.number);
  else if (this_intros_key->typ == STRING)
    log_printf("[I] cfg parsed [%s.%s] (%s)\n", table, k, *k_addr.string);
  else
    log_printf("[I] cfg parsed [%s.%s]\n", table, k);

  return ret;
}
// NOLINTEND(readability-identifier-length,readability-function-cognitive-complexity)

int parse_config(struct config* NNULLABLE config, char* NNULLABLE path) {
  FILE* fd = fopen(path, "r");
  if (fd == NULL) {
    log_perror("fopen");
    log_printf(" [I] No config, place one at " LIDM_CONF_PATH
               " or set the LIDM_CONF env variable");
    return 0; // Its fine now anyways
  }

  bool ret = read_desktop(fd, config, config_line_handler);
  (void)fclose(fd);

  if (!ret) return -1;
  return 0;
}
