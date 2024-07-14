#include <pwd.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#include <sessions.h>
#include <ui.h>
#include <users.h>

static const struct config default_config() {
  struct theme_colors __colors;
  __colors.bg = "48;2;38;28;28";
  __colors.fg = "22;24;38;2;245;245;245";
  __colors.err = "1;31";
  __colors.s_wl = "38;2;255;174;66";
  __colors.s_xorg = "38;2;37;175;255";
  __colors.s_shell = "38;2;34;140;34";
  __colors.f_other = "38;2;255;64;64";
  __colors.e_hostname = "38;2;255;64;64";
  __colors.e_date = "38;2;144;144;144";
  __colors.e_box = "38;2;122;122;122";
  __colors.e_header = "4;38;2;0;255;0";
  __colors.e_user = "36";
  __colors.e_passwd = "4;38;2;245;245;205";
  __colors.e_badpasswd = "3;4;31";
  __colors.e_key = "38;2;255;174;66";

  struct theme_chars __chars;
  __chars.hb = "─";
  __chars.vb = "│";
  __chars.ctl = "┌";
  __chars.ctr = "┐";
  __chars.cbl = "└";
  __chars.cbr = "┘";

  struct theme __theme;
  __theme.colors = __colors;
  __theme.chars = __chars;

  struct functions __functions;
  __functions.poweroff = F1;
  __functions.reboot = F2;
  __functions.refresh = F5;

  struct strings __strings;
  __strings.f_poweroff = "powewoff";
  __strings.f_reboot = "rewoot";
  __strings.f_refresh = "rewresh";
  __strings.e_user = "wuser";
  __strings.e_passwd = "passwd";
  __strings.s_xorg = "xworg";
  __strings.s_wayland = "waywand";
  __strings.s_shell = "swell";

  struct behavior __behavior;
  __behavior.include_defshell = true;

  struct config __config;
  __config.theme = __theme;
  __config.functions = __functions;
  __config.strings = __strings;
  __config.behavior = __behavior;

  return __config;
}

#include <stdio.h>
#include <auth.h>
int main(int argc, char *argv[]) {
  setup(default_config());

  struct users_list *users = get_human_users();
  struct sessions_list *sessions = get_avaliable_sessions();

  int ret = load(users, sessions);
  if (ret == 0)
    execl(argv[0], argv[0], NULL);
}
