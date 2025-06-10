#include <errno.h>
#include <linux/fs.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "config.h"
#include "util.h"

// Alr so ima explain the bitfield returned by `cb` a bit
// 4 bits:
//  0b0001: break out of parsing (returning true)
//  0b0010: free the value
//  0b0100: free the key
//  0b1000: break out of parsing (returning false)
//
// This would return true if everything goes fine, false otherwise (malloc
// error, broke parsing, etc)
// NOLINTBEGIN(modernize-macro-to-enum)
#define LN_BREAK_OK 0b0001
#define LN_FREE_VALUE 0b0010
#define LN_FREE_KEY 0b0100
#define LN_FREE_KV (LN_FREE_KEY | LN_FREE_VALUE)
#define LN_BREAK_ERR 0b1000
// NOLINTEND(modernize-macro-to-enum)
bool line_parser(FILE* fd, u_char (*cb)(char* key, char* value)) {
  bool ok = false;

  char* buf = NULL;
  size_t alloc_size = 0;
  size_t read_size;
  while ((read_size = getline(&buf, &alloc_size, fd)) != -1) {
    ok = true;
    char* key = malloc(read_size);
    if (key == NULL) {
      ok = false;
      break;
    }
    char* value = malloc(read_size);
    if (value == NULL) {
      free(key);
      ok = false;
      break;
    }
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
    if (sscanf(buf, "%[^ ] = %[^\n]\n", key, value) == 2) {
      u_char ret = cb(key, value);
      if (ret & LN_FREE_KEY) free(key);
      if (ret & LN_FREE_VALUE) free(value);
      if (ret & LN_BREAK_ERR) {
        ok = false;
        break;
      }
      if (ret & LN_BREAK_OK) {
        break;
      }
    } else {
      free(key);
      free(value);
    }
  }

  if (buf != NULL) free(buf);
  return ok;
}

struct config* g_config;
// Yanderedev code (wanna fix this with a table or smth)
// NOLINTNEXTLINE(readability-identifier-length,readability-function-cognitive-complexity)
u_char config_line_handler(char* k, char* v) {
  // NOLINTNEXTLINE(readability-function-cognitive-complexity)
  if (strcmp(k, "colors.bg") == 0)
    g_config->theme.colors.bg = v;
  else if (strcmp(k, "colors.fg") == 0)
    g_config->theme.colors.fg = v;
  else if (strcmp(k, "colors.err") == 0)
    g_config->theme.colors.err = v;
  else if (strcmp(k, "colors.s_wayland") == 0)
    g_config->theme.colors.s_wayland = v;
  else if (strcmp(k, "colors.s_xorg") == 0)
    g_config->theme.colors.s_xorg = v;
  else if (strcmp(k, "colors.s_shell") == 0)
    g_config->theme.colors.s_shell = v;
  else if (strcmp(k, "colors.e_hostname") == 0)
    g_config->theme.colors.e_hostname = v;
  else if (strcmp(k, "colors.e_date") == 0)
    g_config->theme.colors.e_date = v;
  else if (strcmp(k, "colors.e_box") == 0)
    g_config->theme.colors.e_box = v;
  else if (strcmp(k, "colors.e_header") == 0)
    g_config->theme.colors.e_header = v;
  else if (strcmp(k, "colors.e_user") == 0)
    g_config->theme.colors.e_user = v;
  else if (strcmp(k, "colors.e_passwd") == 0)
    g_config->theme.colors.e_passwd = v;
  else if (strcmp(k, "colors.e_badpasswd") == 0)
    g_config->theme.colors.e_badpasswd = v;
  else if (strcmp(k, "colors.e_key") == 0)
    g_config->theme.colors.e_key = v;
  else if (strcmp(k, "chars.hb") == 0)
    g_config->theme.chars.hb = v;
  else if (strcmp(k, "chars.vb") == 0)
    g_config->theme.chars.vb = v;
  else if (strcmp(k, "chars.ctl") == 0)
    g_config->theme.chars.ctl = v;
  else if (strcmp(k, "chars.ctr") == 0)
    g_config->theme.chars.ctr = v;
  else if (strcmp(k, "chars.cbl") == 0)
    g_config->theme.chars.cbl = v;
  else if (strcmp(k, "chars.cbr") == 0)
    g_config->theme.chars.cbr = v;
  else if (strcmp(k, "functions.poweroff") == 0) {
    g_config->functions.poweroff = find_keyname(v);
    return LN_FREE_KV;
  } else if (strcmp(k, "functions.reboot") == 0) {
    g_config->functions.reboot = find_keyname(v);
    return LN_FREE_KV;
  } else if (strcmp(k, "functions.refresh") == 0) {
    g_config->functions.refresh = find_keyname(v);
    return LN_FREE_KV;
  } else if (strcmp(k, "strings.f_poweroff") == 0)
    g_config->strings.f_poweroff = v;
  else if (strcmp(k, "strings.f_reboot") == 0)
    g_config->strings.f_reboot = v;
  else if (strcmp(k, "strings.f_refresh") == 0)
    g_config->strings.f_refresh = v;
  else if (strcmp(k, "strings.e_user") == 0)
    g_config->strings.e_user = v;
  else if (strcmp(k, "strings.e_passwd") == 0)
    g_config->strings.e_passwd = v;
  else if (strcmp(k, "strings.s_wayland") == 0)
    g_config->strings.s_wayland = v;
  else if (strcmp(k, "strings.s_xorg") == 0)
    g_config->strings.s_xorg = v;
  else if (strcmp(k, "strings.s_shell") == 0)
    g_config->strings.s_shell = v;
  else if (strcmp(k, "behavior.include_defshell") == 0) {
    g_config->behavior.include_defshell = strcmp(v, "true") == 0;
    return LN_FREE_KV;
  } else if (strcmp(k, "behavior.source") == 0)
    vec_push(&g_config->behavior.source, v);
  else if (strcmp(k, "behavior.user_source") == 0)
    vec_push(&g_config->behavior.user_source, v);
  else
    return LN_BREAK_ERR | LN_FREE_KV;

  return LN_FREE_KEY;
}

struct config* parse_config(char* path) {
  // struct stat sb;
  errno = 0;
  FILE* fd = fopen(path, "r");
  if (fd == NULL) {
    perror("fopen");
    (void)fputs(
        "Please place a config file at /etc/lidm.ini or set the LIDM_CONF "
        "env variable",
        stderr);
    return NULL;
  }
  // if(stat(path, &sb) != 0) {
  //   perror("stat");
  // }

  g_config = malloc(sizeof(struct config));
  g_config->behavior.source = vec_new();
  g_config->behavior.user_source = vec_new();

  if (g_config == NULL) return NULL;
  bool ret = line_parser(fd, config_line_handler);
  (void)fclose(fd);
  if (!ret) {
    free(g_config);
    return NULL;
  }

  return g_config;
}
