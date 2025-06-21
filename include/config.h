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
static const char* NNULLABLE const INTROS_TYS_NAMES[] = {
    [STRING] = "STRING",
    [BOOL] = "BOOL",
    [KEY] = "KEY",
    [STRING_ARRAY] = "STRING_ARRAY",
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
#define BUILD_DEFAULT(utable, table, TABLE)                    \
  static const struct table_##table DEFAULT_TABLE_##utable = { \
      TABLE(DEFAULT_BUILDER, table)}
#define BUILD_INTROS(utable, table, TABLE)                           \
  static const struct introspection_item INTROS_TABLE_##utable[] = { \
      TABLE(INTROS_BUILDER, table)}

#define BUILD(table, utable, TABLE)    \
  BUILD_TABLE(table, TABLE);           \
  BUILD_DEFAULT(utable, table, TABLE); \
  BUILD_INTROS(utable, table, TABLE);

// should be ansi escape codes under \x1b[...m
// if not prepared accordingly, it might break
#define TABLE_COLORS(F, name)                                      \
  F(char* NNULLABLE, bg, STRING, "48;2;38;28;28", name)            \
  F(char* NNULLABLE, fg, STRING, "22;24;38;2;245;245;245", name)   \
  F(char* NNULLABLE, err, STRING, "1;31", name)                    \
  F(char* NNULLABLE, s_wayland, STRING, "38;2;255;174;66", name)   \
  F(char* NNULLABLE, s_xorg, STRING, "38;2;37;175;255", name)      \
  F(char* NNULLABLE, s_shell, STRING, "38;2;34;140;34", name)      \
  F(char* NNULLABLE, e_hostname, STRING, "38;2;255;64;64", name)   \
  F(char* NNULLABLE, e_date, STRING, "38;2;144;144;144", name)     \
  F(char* NNULLABLE, e_box, STRING, "38;2;122;122;122", name)      \
  F(char* NNULLABLE, e_header, STRING, "4;38;2;0;255;0", name)     \
  F(char* NNULLABLE, e_user, STRING, "36", name)                   \
  F(char* NNULLABLE, e_passwd, STRING, "4;38;2;245;245;205", name) \
  F(char* NNULLABLE, e_badpasswd, STRING, "3;4;31", name)          \
  F(char* NNULLABLE, e_key, STRING, "38;2;255;174;66", name)

BUILD(colors, COLORS, TABLE_COLORS);

// even if they're multiple bytes long
// they should only take up 1 char size on display
#define TABLE_CHARS(F, name)                 \
  F(char* NNULLABLE, hb, STRING, "─", name)  \
  F(char* NNULLABLE, vb, STRING, "│", name)  \
  F(char* NNULLABLE, ctl, STRING, "┌", name) \
  F(char* NNULLABLE, ctr, STRING, "┐", name) \
  F(char* NNULLABLE, cbl, STRING, "└", name) \
  F(char* NNULLABLE, cbr, STRING, "┘", name)

BUILD(chars, CHARS, TABLE_CHARS);

#define TABLE_FUNCTIONS(F, name)        \
  F(enum keys, poweroff, KEY, F1, name) \
  F(enum keys, reboot, KEY, F2, name)   \
  F(enum keys, refresh, KEY, F5, name)

BUILD(functions, FUNCTIONS, TABLE_FUNCTIONS);

#define TABLE_STRINGS(F, name)                             \
  F(char* NNULLABLE, f_poweroff, STRING, "poweroff", name) \
  F(char* NNULLABLE, f_reboot, STRING, "reboot", name)     \
  F(char* NNULLABLE, f_refresh, STRING, "refresh", name)   \
  F(char* NNULLABLE, e_user, STRING, "user", name)         \
  F(char* NNULLABLE, e_passwd, STRING, "password", name)   \
  F(char* NNULLABLE, s_wayland, STRING, "wayland", name)   \
  F(char* NNULLABLE, s_xorg, STRING, "xorg", name)         \
  F(char* NNULLABLE, s_shell, STRING, "shell", name)       \
  F(char* NNULLABLE, opts_pre, STRING, "< ", name)         \
  F(char* NNULLABLE, opts_post, STRING, " >", name)

BUILD(strings, STRINGS, TABLE_STRINGS);

#define NULL_VEC    \
  (struct Vector) { \
    0, 0, NULL      \
  }
#define TABLE_BEHAVIOR(F, name)                               \
  F(bool, include_defshell, BOOL, true, name)                 \
  F(struct Vector, source, STRING_ARRAY, NULL_VEC, name)      \
  F(struct Vector, user_source, STRING_ARRAY, NULL_VEC, name) \
  F(char* NNULLABLE, timefmt, STRING, "%X %x", name)

BUILD(behavior, BEHAVIOR, TABLE_BEHAVIOR);

//// CONFIG
struct config {
  struct table_colors colors;
  struct table_chars chars;
  struct table_functions functions;
  struct table_strings strings;
  struct table_behavior behavior;
};

static const struct config DEFAULT_CONFIG = {
    .colors = DEFAULT_TABLE_COLORS,
    .chars = DEFAULT_TABLE_CHARS,
    .functions = DEFAULT_TABLE_FUNCTIONS,
    .strings = DEFAULT_TABLE_STRINGS,
    .behavior = DEFAULT_TABLE_BEHAVIOR,
};

struct introspection_table {
  char* NNULLABLE tname;
  size_t offset;
  const struct introspection_item* NNULLABLE intros;
  size_t len;
};

static const struct introspection_table CONFIG_INSTROSPECTION[] = {
    {"colors", offsetof(struct config, colors), INTROS_TABLE_COLORS,
     sizeof(INTROS_TABLE_COLORS) / sizeof(INTROS_TABLE_COLORS[0])},
    {"chars", offsetof(struct config, chars), INTROS_TABLE_CHARS,
     sizeof(INTROS_TABLE_CHARS) / sizeof(INTROS_TABLE_CHARS[0])},
    {"functions", offsetof(struct config, functions), INTROS_TABLE_FUNCTIONS,
     sizeof(INTROS_TABLE_FUNCTIONS) / sizeof(INTROS_TABLE_FUNCTIONS[0])},
    {"strings", offsetof(struct config, strings), INTROS_TABLE_STRINGS,
     sizeof(INTROS_TABLE_STRINGS) / sizeof(INTROS_TABLE_STRINGS[0])},
    {"behavior", offsetof(struct config, behavior), INTROS_TABLE_BEHAVIOR,
     sizeof(INTROS_TABLE_BEHAVIOR) / sizeof(INTROS_TABLE_BEHAVIOR[0])},
};

//// FUNCTIONS
int parse_config(struct config* NNULLABLE config, char* NNULLABLE path);

#endif
