// i'm sorry
// really sorry

#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "auth.h"
#include "efield.h"
#include "keys.h"
#include "ofield.h"
#include "sessions.h"
#include "ui.h"
#include "ui_state.h"
#include "users.h"
#include "util.h"

const u_char inputs_n = 3;
const uint boxw = 50;
const uint boxh = 12;

static void print_box();
static void print_footer();
static void restore_all();
static void signal_handler(int);

struct uint_point {
  uint x;
  uint y;
};

static void print_session(struct uint_point, struct session, bool);
static void print_user(struct uint_point, struct user, bool);
static void print_passwd(struct uint_point, uint, bool);

// ansi resource: https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
static struct termios orig_term;
static struct termios term;
static struct winsize window;

struct config* g_config = NULL;
void setup(struct config* config) {
  g_config = config;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &window);

  // 2 padding top and bottom for footer and vertical compensation
  // 2 padding left & right to not overflow footer width
  if (window.ws_row < boxh + 4 || window.ws_col < boxw + 4) {
    (void)fprintf(stderr, "\x1b[1;31mScreen too small\x1b[0m\n");
    exit(1);
  }

  tcgetattr(STDOUT_FILENO, &orig_term);
  term = orig_term; // save term
  // "stty" attrs
  term.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDOUT_FILENO, TCSANOW, &term);

  // save cursor pos, save screen, set color and reset screen
  // (applying color to all screen)
  printf("\x1b[s\x1b[?47h\x1b[%s;%sm\x1b[2J", g_config->colors.bg,
         g_config->colors.fg);

  print_footer();
  (void)atexit(restore_all);
  (void)signal(SIGINT, signal_handler);
}

static struct uint_point box_start() {
  return (struct uint_point){
      .x = ((window.ws_col - boxw) / 2) + 1,
      .y = ((window.ws_row - boxh) / 2) + 1,
  };
}

static char* fmt_time() {
  time_t tme = time(NULL);
  struct tm tm = *localtime(&tme);

  // TODO: use strftime and a cfg template string
  char* buf;
  asprintf(&buf, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900,
           tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

  return buf;
}

void ui_update_cursor_focus() {
  struct uint_point bstart = box_start();
  u_char line = bstart.y;
  u_char col = bstart.x + 15;

  struct opts_field* ofield = get_opts_ffield();
  col += ofield_display_cursor_col(ofield);
  if (ofield->opts > 1) col += utf8len(g_config->strings.opts_pre);

  // rows in here quite bodged
  if (focused_input == SESSION) {
    line += 5;
  } else if (focused_input == USER) {
    line += 7;
  } else if (focused_input == PASSWD)
    line += 9;

  (void)printf("\x1b[%d;%dH", line, col);
  (void)fflush(stdout);
}

void ui_update_field(enum input focused_input) {
  struct uint_point origin = box_start();

  if (focused_input == PASSWD) {
    print_passwd(origin, utf8len(of_passwd.efield.content), false);
  } else if (focused_input == SESSION) {
    print_session(origin, st_session(g_config->behavior.include_defshell),
                  of_session.opts > 1);
  } else if (focused_input == USER) {
    print_user(origin, st_user(), of_user.opts > 1);
    ui_update_field(SESSION);
  }

  ui_update_cursor_focus();
}

void ui_update_ffield() {
  ui_update_field(focused_input);
}

void ui_update_ofield(struct opts_field* self) {
  enum input input;
  if (self == &of_session)
    input = SESSION;
  else if (self == &of_user)
    input = USER;
  else if (self == &of_passwd)
    input = PASSWD;
  else
    return;

  ui_update_field(input);
}

static char* unknown_str = "unknown";
int load(struct Vector* users, struct Vector* sessions) {
  /// SETUP
  gusers = users;
  gsessions = sessions;

  // hostnames larger won't render properly
  char* hostname = malloc(16);
  if (gethostname(hostname, 16) != 0) {
    free(hostname);
    hostname = unknown_str;
  } else {
    hostname[15] = '\0';
  }

  of_session =
      ofield_new(sessions->length + g_config->behavior.include_defshell);
  of_user = ofield_new(users->length);
  of_passwd = ofield_new(0);

  /// PRINTING
  const struct uint_point boxstart = box_start();

  // printf box
  print_box();

  // put hostname
  printf("\x1b[%d;%dH\x1b[%sm%s\x1b[%sm", boxstart.y + 2,
         boxstart.x + 12 - (uint)strlen(hostname), g_config->colors.e_hostname,
         hostname, g_config->colors.fg);
  if (hostname != unknown_str) free(hostname);

  // put date
  char* fmtd_time = fmt_time();
  printf("\x1b[%d;%dH\x1b[%sm%s\x1b[%sm", boxstart.y + 2,
         boxstart.x + boxw - 3 - (uint)strlen(fmtd_time),
         g_config->colors.e_date, fmtd_time, g_config->colors.fg);
  free(fmtd_time);

  ui_update_field(SESSION);
  ui_update_field(USER);
  ui_update_field(PASSWD);
  ui_update_cursor_focus();

  /// INTERACTIVE
  u_char len;
  char seq[256];
  uint esc = 0;
  while (true) {
    read_press(&len, seq);
    if (*seq == '\x1b') {
      enum keys ansi_code = find_ansi(seq);
      if (ansi_code != -1) {
        if (ansi_code == ESC) {
          esc = 2;
        } else if (ansi_code == g_config->functions.refresh) {
          restore_all();
          return 0;
        } else if (ansi_code == g_config->functions.reboot) {
          restore_all();
          reboot(RB_AUTOBOOT);
          exit(0);
        } else if (ansi_code == g_config->functions.poweroff) {
          restore_all();
          reboot(RB_POWER_OFF);
          exit(0);
        } else if (ansi_code == A_UP || ansi_code == A_DOWN) {
          st_ch_focus(ansi_code == A_DOWN ? 1 : -1);
        } else if (ansi_code == A_RIGHT || ansi_code == A_LEFT) {
          if (esc)
            st_ch_of_opts(ansi_code == A_RIGHT ? 1 : -1);
          else
            st_ch_ef_col(ansi_code == A_RIGHT ? 1 : -1);
        }
      }
    } else {
      if (len == 1 && *seq == '\n') {
        if (!launch(st_user().username, of_passwd.efield.content,
                    st_session(g_config->behavior.include_defshell),
                    &restore_all, g_config)) {
          print_passwd(box_start(), utf8len(of_passwd.efield.content), true);
          ui_update_cursor_focus();
        }
      } else
        st_kbd_type(seq, g_config->behavior.include_defshell);
    }

    if (esc != 0) esc--;
  }
}

static char* line_cleaner = NULL;
static void clean_line(struct uint_point origin, uint line) {
  if (line_cleaner == NULL) {
    line_cleaner = malloc((boxw - 2) * sizeof(char) + 1);
    memset(line_cleaner, 32, boxw - 2);
    line_cleaner[boxw - 2] = 0;
  }
  printf("\x1b[%d;%dH", origin.y + line, origin.x + 1);
  printf("%s", line_cleaner);
}

// TODO: session_len > 32
static void print_session(struct uint_point origin, struct session session,
                          bool multiple) {
  clean_line(origin, 5);
  const char* session_type;
  if (session.type == XORG) {
    session_type = g_config->strings.s_xorg;
  } else if (session.type == WAYLAND) {
    session_type = g_config->strings.s_wayland;
  } else {
    session_type = g_config->strings.s_shell;
  }
  printf("\r\x1b[%luC\x1b[%sm%s\x1b[%sm",
         (ulong)(origin.x + 11 - strlen(session_type)),
         g_config->colors.e_header, session_type, g_config->colors.fg);

  char* session_color;
  if (session.type == XORG) {
    session_color = g_config->colors.s_xorg;
  } else if (session.type == WAYLAND) {
    session_color = g_config->colors.s_wayland;
  } else {
    session_color = g_config->colors.s_shell;
  }

  if (multiple) {
    printf("\r\x1b[%dC%s\x1b[%sm%s\x1b[%sm%s", origin.x + 14,
           g_config->strings.opts_pre, session_color, session.name,
           g_config->colors.fg, g_config->strings.opts_post);
  } else {
    printf("\r\x1b[%dC\x1b[%sm%s\x1b[%sm", origin.x + 14, session_color,
           session.name, g_config->colors.fg);
  }
}

// TODO: user_len > 32
static void print_user(struct uint_point origin, struct user user,
                       bool multiple) {
  clean_line(origin, 7);
  printf("\r\x1b[%luC\x1b[%sm%s\x1b[%sm",
         (ulong)(origin.x + 11 - strlen(g_config->strings.e_user)),
         g_config->colors.e_header, g_config->strings.e_user,
         g_config->colors.fg);

  char* user_color = g_config->colors.e_user;

  if (multiple) {
    printf("\r\x1b[%dC< \x1b[%sm%s\x1b[%sm >", origin.x + 14, user_color,
           user.display_name, g_config->colors.fg);
  } else {
    printf("\r\x1b[%dC\x1b[%sm%s\x1b[%sm", origin.x + 14, user_color,
           user.display_name, g_config->colors.fg);
  }
}

static char passwd_prompt[33];
// TODO: passwd_len > 32
static void print_passwd(struct uint_point origin, uint length, bool err) {
  clean_line(origin, 9);
  printf("\r\x1b[%luC\x1b[%sm%s\x1b[%sm",
         (ulong)(origin.x + 11 - strlen(g_config->strings.e_passwd)),
         g_config->colors.e_header, g_config->strings.e_passwd,
         g_config->colors.fg);

  char* pass_color;
  if (err)
    pass_color = g_config->colors.e_badpasswd;
  else
    pass_color = g_config->colors.e_passwd;

  ulong prompt_len = sizeof(passwd_prompt);
  ulong actual_len = length > prompt_len ? prompt_len : length;
  memset(passwd_prompt, ' ', prompt_len);
  memset(passwd_prompt, '*', actual_len);
  passwd_prompt[32] = 0;

  printf("\r\x1b[%dC\x1b[%sm", origin.x + 14, pass_color);
  printf("%s", passwd_prompt);

  printf("\x1b[%sm", g_config->colors.fg);
}

static void print_empty_row(uint w, uint n, char* edge1, char* edge2) {
  for (size_t i = 0; i < n; i++) {
    printf("%s\x1b[%dC%s\x1b[%dD\x1b[1B", edge1, w, edge2, w + 2);
  }
}

static void print_row(uint w, uint n, char* edge1, char* edge2, char* filler) {
  for (size_t i = 0; i < n; i++) {
    printf("%s", edge1);
    for (size_t i = 0; i < w; i++) {
      printf("%s", filler);
    }
    printf("%s\x1b[%dD\x1b[1B", edge2, w + 2);
  }
}

static void print_box() {
  // TODO: check min sizes
  const struct uint_point bstart = box_start();

  printf("\x1b[%d;%dH\x1b[%sm", bstart.y, bstart.x, g_config->colors.e_box);
  fflush(stdout);
  print_row(boxw - 2, 1, g_config->chars.ctl, g_config->chars.ctr,
            g_config->chars.hb);
  print_empty_row(boxw - 2, boxh - 2, g_config->chars.vb, g_config->chars.vb);
  print_row(boxw - 2, 1, g_config->chars.cbl, g_config->chars.cbr,
            g_config->chars.hb);
  printf("\x1b[%sm", g_config->colors.fg);
  fflush(stdout);
}

static void print_footer() {
  size_t bsize = snprintf(
      NULL, 0, "%s %s  %s %s  %s %s", g_config->strings.f_poweroff,
      key_names[g_config->functions.poweroff], g_config->strings.f_reboot,
      key_names[g_config->functions.reboot], g_config->strings.f_refresh,
      key_names[g_config->functions.refresh]);

  uint row = window.ws_row - 1;
  uint col = window.ws_col - 2 - bsize;
  printf(
      "\x1b[%3$d;%4$dH%8$s \x1b[%1$sm%5$s\x1b[%2$sm  %9$s "
      "\x1b[%1$sm%6$s\x1b[%2$sm  %10$s \x1b[%1$sm%7$s\x1b[%2$sm",
      g_config->colors.e_key, g_config->colors.fg, row, col,
      key_names[g_config->functions.poweroff],
      key_names[g_config->functions.reboot],
      key_names[g_config->functions.refresh], g_config->strings.f_poweroff,
      g_config->strings.f_reboot, g_config->strings.f_refresh);
  fflush(stdout);
}

void print_err(const char* msg) {
  struct uint_point origin = box_start();
  fprintf(stderr, "\x1b[%d;%dH%s(%d): %s", origin.y - 1, origin.x, msg, errno,
          strerror(errno));
}

void print_errno(const char* descr) {
  struct uint_point origin = box_start();
  if (descr == NULL)
    fprintf(stderr, "\x1b[%d;%dH\x1b[%smunknown error(%d): %s", origin.y - 1,
            origin.x, g_config->colors.err, errno, strerror(errno));
  else {
    fprintf(stderr, "\x1b[%d;%dH\x1b[%sm%s(%d): %s", origin.y - 1, origin.x,
            g_config->colors.err, descr, errno, strerror(errno));
  }
}

void restore_all() {
  // restore cursor pos, restore screen and show cursor
  printf("\x1b[u\x1b[?47l\x1b[?25h");
  fflush(stdout);
  tcsetattr(STDOUT_FILENO, TCSANOW, &orig_term);
}

void signal_handler(int code) {
  restore_all();
  exit(code);
}
