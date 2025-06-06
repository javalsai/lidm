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
// This would return true if everything goes fine, false otherwise (malloc error, broke parsing, etc)
bool line_parser(FILE *fd, ssize_t *blksize,
                 u_char (*cb)(char *key, char *value)) {
  size_t opt_size = 4096;
  if (blksize != NULL)
    opt_size = *blksize;

  while (true) {
    size_t alloc_size = opt_size;
    char *buf = malloc(alloc_size);
    if (buf == NULL)
      return false;
    ssize_t read_size = getline(&buf, &alloc_size, fd);
    if (read_size == -1) {
      free(buf);
      break;
    }

    uint read;
    char *key = malloc(read_size);
    if (key == NULL) {
      free(buf);
      return false;
    }
    char *value = malloc(read_size);
    if (value == NULL) {
      free(buf);
      return false;
    }
    if ((read = sscanf(buf, "%[^ ] = %[^\n]\n", key, value)) != 0) {
      u_char ret = cb(key, value);
      if (ret & 0b0100)
        free(key);
      if (ret & 0b0010)
        free(value);
      if (ret & 0b1000) {
        free(buf);
        return false;
      }
      if (ret & 0b0001) {
        free(buf);
        break;
      }
    }
    free(buf);
  }

  return true;
}

struct config *__config;
// Yanderedev code (wanna fix this with a table or smth)
u_char config_line_handler(char *k, char *v) {
  if (strcmp(k, "colors.bg") == 0)
    __config->theme.colors.bg = v;
  else if (strcmp(k, "colors.fg") == 0)
    __config->theme.colors.fg = v;
  else if (strcmp(k, "colors.err") == 0)
    __config->theme.colors.err = v;
  else if (strcmp(k, "colors.s_wayland") == 0)
    __config->theme.colors.s_wayland = v;
  else if (strcmp(k, "colors.s_xorg") == 0)
    __config->theme.colors.s_xorg = v;
  else if (strcmp(k, "colors.s_shell") == 0)
    __config->theme.colors.s_shell = v;
  else if (strcmp(k, "colors.e_hostname") == 0)
    __config->theme.colors.e_hostname = v;
  else if (strcmp(k, "colors.e_date") == 0)
    __config->theme.colors.e_date = v;
  else if (strcmp(k, "colors.e_box") == 0)
    __config->theme.colors.e_box = v;
  else if (strcmp(k, "colors.e_header") == 0)
    __config->theme.colors.e_header = v;
  else if (strcmp(k, "colors.e_user") == 0)
    __config->theme.colors.e_user = v;
  else if (strcmp(k, "colors.e_passwd") == 0)
    __config->theme.colors.e_passwd = v;
  else if (strcmp(k, "colors.e_badpasswd") == 0)
    __config->theme.colors.e_badpasswd = v;
  else if (strcmp(k, "colors.e_key") == 0)
    __config->theme.colors.e_key = v;
  else if (strcmp(k, "chars.hb") == 0)
    __config->theme.chars.hb = v;
  else if (strcmp(k, "chars.vb") == 0)
    __config->theme.chars.vb = v;
  else if (strcmp(k, "chars.ctl") == 0)
    __config->theme.chars.ctl = v;
  else if (strcmp(k, "chars.ctr") == 0)
    __config->theme.chars.ctr = v;
  else if (strcmp(k, "chars.cbl") == 0)
    __config->theme.chars.cbl = v;
  else if (strcmp(k, "chars.cbr") == 0)
    __config->theme.chars.cbr = v;
  else if (strcmp(k, "functions.poweroff") == 0) {
    __config->functions.poweroff = find_keyname(v);
    return 0b0110;
  } else if (strcmp(k, "functions.reboot") == 0) {
    __config->functions.reboot = find_keyname(v);
    return 0b0110;
  } else if (strcmp(k, "functions.refresh") == 0) {
    __config->functions.refresh = find_keyname(v);
    return 0b0110;
  } else if (strcmp(k, "strings.f_poweroff") == 0)
    __config->strings.f_poweroff = v;
  else if (strcmp(k, "strings.f_reboot") == 0)
    __config->strings.f_reboot = v;
  else if (strcmp(k, "strings.f_refresh") == 0)
    __config->strings.f_refresh = v;
  else if (strcmp(k, "strings.e_user") == 0)
    __config->strings.e_user = v;
  else if (strcmp(k, "strings.e_passwd") == 0)
    __config->strings.e_passwd = v;
  else if (strcmp(k, "strings.s_wayland") == 0)
    __config->strings.s_wayland = v;
  else if (strcmp(k, "strings.s_xorg") == 0)
    __config->strings.s_xorg = v;
  else if (strcmp(k, "strings.s_shell") == 0)
    __config->strings.s_shell = v;
  else if (strcmp(k, "behavior.include_defshell") == 0) {
    __config->behavior.include_defshell = strcmp(v, "true") == 0;
    return 0b0110;
  } else if (strcmp(k, "behavior.source") == 0)
    vec_push(&__config->behavior.source, v);
  else if (strcmp(k, "behavior.user_source") == 0)
    vec_push(&__config->behavior.user_source, v);
  else
    return 0b1111;

  return 0b0100;
}

struct config *parse_config(char *path) {
  struct stat sb;
  FILE *fd = fopen(path, "r");
  if (fd == NULL || (stat(path, &sb) == -1)) {
    perror("fopen");
    return NULL;
  }

  __config = malloc(sizeof(struct config));
  __config->behavior.source = vec_new();
  __config->behavior.user_source = vec_new();

  if (__config == NULL)
    return NULL;
  bool ret = line_parser(fd, (ssize_t *)&sb.st_blksize, config_line_handler);
  if (!ret) {
    free(__config);
    return NULL;
  }

  return __config;
}
