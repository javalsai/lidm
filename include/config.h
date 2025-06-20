#ifndef CONFIGH_
#define CONFIGH_

#include <stdbool.h>
#include <stddef.h>

#include "keys.h"
#include "macros.h"
#include "util.h"

enum introspection_type {
  STRING,
  BOOL,
  KEY,
  STRING_ARRAY,
};
static const char* const intros_tys_names[] = {
    [STRING] = "STRING",
    [BOOL] = "BOOL",
    [KEY] = "KEY",
    [STRING_ARRAY] = "STRING_ARRAY",
};

union introspection_variant {
  char* string;
  bool boolean;
  enum keys key;
  char** string_array;
};

struct introspection_item {
  char* NNULLABLE name;
  size_t offset;
  enum introspection_type typ;
};

#define INTROS_ITEM(key, table, ty)   \
  {                                   \
      .name = #key,                   \
      .offset = offsetof(table, key), \
      .typ = (ty),                    \
  }

#define STRUCT_BUILDER(cty, name, ty, def, tname) cty name;
#define DEFAULT_BUILDER(cty, name, ty, def, tname) .name = (def),
#define INTROS_BUILDER(cty, name, ty, def, tname) \
  INTROS_ITEM(name, struct table_##tname, ty),

#define BUILD_TABLE(table, TABLE) \
  struct table_##table {          \
    TABLE(STRUCT_BUILDER, table)  \
  }
#define BUILD_DEFAULT(table, TABLE)                           \
  static const struct table_##table default_table_##table = { \
      TABLE(DEFAULT_BUILDER, table)}
#define BUILD_INTROS(table, TABLE)                                  \
  static const struct introspection_item intros_table_##table[] = { \
      TABLE(INTROS_BUILDER, table)}

#define BUILD(table, TABLE)    \
  BUILD_TABLE(table, TABLE);   \
  BUILD_DEFAULT(table, TABLE); \
  BUILD_INTROS(table, TABLE);

// should be ansi escape codes under \x1b[...m
// if not prepared accordingly, it might break
#define TABLE_COLORS(F, name)                            \
  F(char*, bg, STRING, "48;2;38;28;28", name)            \
  F(char*, fg, STRING, "22;24;38;2;245;245;245", name)   \
  F(char*, err, STRING, "1;31", name)                    \
  F(char*, s_wayland, STRING, "38;2;255;174;66", name)   \
  F(char*, s_xorg, STRING, "38;2;37;175;255", name)      \
  F(char*, s_shell, STRING, "38;2;34;140;34", name)      \
  F(char*, e_hostname, STRING, "38;2;255;64;64", name)   \
  F(char*, e_date, STRING, "38;2;144;144;144", name)     \
  F(char*, e_box, STRING, "38;2;122;122;122", name)      \
  F(char*, e_header, STRING, "4;38;2;0;255;0", name)     \
  F(char*, e_user, STRING, "36", name)                   \
  F(char*, e_passwd, STRING, "4;38;2;245;245;205", name) \
  F(char*, e_badpasswd, STRING, "3;4;31", name)          \
  F(char*, e_key, STRING, "38;2;255;174;66", name)

BUILD(colors, TABLE_COLORS);

// even if they're multiple bytes long
// they should only take up 1 char size on display
#define TABLE_CHARS(F, name)       \
  F(char*, hb, STRING, "─", name)  \
  F(char*, vb, STRING, "│", name)  \
  F(char*, ctl, STRING, "┌", name) \
  F(char*, ctr, STRING, "┐", name) \
  F(char*, cbl, STRING, "└", name) \
  F(char*, cbr, STRING, "┘", name)

BUILD(chars, TABLE_CHARS);

#define TABLE_FUNCTIONS(F, name)        \
  F(enum keys, poweroff, KEY, F1, name) \
  F(enum keys, reboot, KEY, F2, name)   \
  F(enum keys, refresh, KEY, F5, name)

BUILD(functions, TABLE_FUNCTIONS);

#define TABLE_STRINGS(F, name)                   \
  F(char*, f_poweroff, STRING, "poweroff", name) \
  F(char*, f_reboot, STRING, "reboot", name)     \
  F(char*, f_refresh, STRING, "refresh", name)   \
  F(char*, e_user, STRING, "user", name)         \
  F(char*, e_passwd, STRING, "password", name)   \
  F(char*, s_wayland, STRING, "wayland", name)   \
  F(char*, s_xorg, STRING, "xorg", name)         \
  F(char*, s_shell, STRING, "shell", name)       \
  F(char*, opts_pre, STRING, "< ", name)         \
  F(char*, opts_post, STRING, " >", name)

BUILD(strings, TABLE_STRINGS);

#define NULL_VEC    \
  (struct Vector) { \
    0, 0, NULL      \
  }
#define TABLE_BEHAVIOR(F, name)                          \
  F(bool, include_defshell, BOOL, true, name)            \
  F(struct Vector, source, STRING_ARRAY, NULL_VEC, name) \
  F(struct Vector, user_source, STRING_ARRAY, NULL_VEC, name)

BUILD(behavior, TABLE_BEHAVIOR);

//// CONFIG
struct config {
  struct table_colors colors;
  struct table_chars chars;
  struct table_functions functions;
  struct table_strings strings;
  struct table_behavior behavior;
};

static const struct config default_config = {
    .colors = default_table_colors,
    .chars = default_table_chars,
    .functions = default_table_functions,
    .strings = default_table_strings,
    .behavior = default_table_behavior,
};

struct introspection_table {
  char* NNULLABLE tname;
  size_t offset;
  const struct introspection_item* NNULLABLE intros;
  size_t len;
};

static const struct introspection_table config_instrospection[] = {
    {"colors", offsetof(struct config, colors), intros_table_colors,
     sizeof(intros_table_colors) / sizeof(intros_table_colors[0])},
    {"chars", offsetof(struct config, chars), intros_table_chars,
     sizeof(intros_table_chars) / sizeof(intros_table_chars[0])},
    {"functions", offsetof(struct config, functions), intros_table_functions,
     sizeof(intros_table_functions) / sizeof(intros_table_functions[0])},
    {"strings", offsetof(struct config, strings), intros_table_strings,
     sizeof(intros_table_strings) / sizeof(intros_table_strings[0])},
    {"behavior", offsetof(struct config, behavior), intros_table_behavior,
     sizeof(intros_table_behavior) / sizeof(intros_table_behavior[0])},
};

//// FUNCTIONS
int parse_config(struct config* NNULLABLE config, char* NNULLABLE path);

#endif
