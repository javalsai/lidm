// i'm sorry
// really sorry

#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
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
#include "launch_state.h"

const u_char INPUTS_N = 3;

static void print_box();
static void print_footer();
static void restore_all();
static void signal_handler(int code);

struct uint_point {
  uint x;
  uint y;
};

static void print_session(struct uint_point origin, struct session session,
                          bool multiple);
static void print_user(struct uint_point origin, struct user user,
                       bool multiple);
static void print_passwd(struct uint_point origin, uint length, bool err);

// ansi resource: https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
static struct termios orig_term;
static struct termios term;
static struct winsize window;

#define INNER_BOX_OUT_MARGIN 2
struct config* g_config = NULL;
void setup(struct config* config) {
  g_config = config;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &window);

  // at least
  // 2 padding top and bottom for footer and vertical compensation
  // 2 padding left & right to not overflow footer width
  if (window.ws_row < BOX_HEIGHT + INNER_BOX_OUT_MARGIN * 2 ||
      window.ws_col < BOX_WIDTH + INNER_BOX_OUT_MARGIN * 2) {
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
      .x = ((window.ws_col - BOX_WIDTH) / 2) + 1, // looks better
      .y = ((window.ws_row - BOX_HEIGHT) / 2),    // leave more space under
  };
}

#define STRFTIME_PREALLOC 64
#define TM_YEAR_EPOCH 1900
static char* fmt_time(const char* fmt) {
  time_t tme = time(NULL);
  struct tm tm = *localtime(&tme);

  size_t alloc_size = STRFTIME_PREALLOC;
  char* buf = malloc(alloc_size);
  if (!buf) return NULL;
  while (true) {
    if (strftime(buf, alloc_size, fmt, &tm) != 0) return buf;

    alloc_size *= 2;
    char* nbuf = realloc(buf, alloc_size);
    if (!nbuf) {
      free(buf);
      return NULL;
    }
    buf = nbuf;
  }
}

void ui_update_cursor_focus() {
  struct uint_point bstart = box_start();
  u_char line = bstart.y;
  u_char col = bstart.x + VALUES_COL;

  struct opts_field* ofield = get_opts_ffield();
  u_char maxlen = VALUE_MAXLEN;
  if (ofield->opts > 1) {
    maxlen -= utf8len(g_config->strings.opts_pre) +
              utf8len(g_config->strings.opts_post);
  }
  col += ofield_display_cursor_col(ofield, maxlen);
  if (ofield->opts > 1) col += utf8len(g_config->strings.opts_pre);

  // rows in here quite bodged
  if (focused_input == SESSION) {
    line += SESSION_ROW;
  } else if (focused_input == USER) {
    line += USER_ROW;
  } else if (focused_input == PASSWD)
    line += PASSWD_ROW;

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

void ui_update_ofield(struct opts_field* NNULLABLE self) {
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
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
int load(struct Vector* users, struct Vector* sessions) {
  /// SETUP
  gusers = users;
  gsessions = sessions;

  // hostnames larger won't render properly
  const u_char HOSTNAME_SIZE = VALUES_COL - VALUES_SEPR - BOX_HMARGIN;
  char hostname_buf[HOSTNAME_SIZE];
  char* hostname = hostname_buf;
  if (gethostname(hostname_buf, HOSTNAME_SIZE) != 0) {
    hostname = unknown_str;
  } else {
    // Ig "successful completion" doesn't contemplate truncation case, so need
    // to append the unspecified nullbyte

    // char* hidx =
    //     (char*)utf8back(&hostname[VALUES_COL - VALUES_SEPR - BOX_HMARGIN -
    //     1]);
    // *hidx = '\0';
  }

  of_session =
      ofield_new(sessions->length + g_config->behavior.include_defshell);
  of_user = ofield_new(users->length);
  of_passwd = ofield_new(0);

  struct LaunchState initial_state = read_launch_state();
  if(initial_state.user_opt > users->length || initial_state.session_opt > sessions->length + behavior.include_defshell) {
	  initial_state.user_opt = 1;
	  initial_state.session_opt = 1;
  }
  of_user.current_opt = initial_state.user_opt;
  of_session.current_opt = initial_state.session_opt;

  /// PRINTING
  const struct uint_point BOXSTART = box_start();

  // printf box
  print_box();

  // put hostname
  printf("\x1b[%d;%dH\x1b[%sm%s\x1b[%sm", BOXSTART.y + HEAD_ROW,
         BOXSTART.x + VALUES_COL - VALUES_SEPR - (uint)utf8len(hostname),
         g_config->colors.e_hostname, hostname, g_config->colors.fg);

  // put date
  char* fmtd_time = fmt_time(g_config->behavior.timefmt);
  printf("\x1b[%d;%dH\x1b[%sm%s\x1b[%sm", BOXSTART.y + HEAD_ROW,
         BOXSTART.x + BOX_WIDTH - 1 - BOX_HMARGIN - (uint)utf8len(fmtd_time),
         g_config->colors.e_date, fmtd_time, g_config->colors.fg);
  free(fmtd_time);

  ui_update_field(SESSION);
  ui_update_field(USER);
  ui_update_field(PASSWD);
  ui_update_cursor_focus();

  /// INTERACTIVE
  u_char len;
  char seq[0xff];
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
        struct LaunchState ls;
		ls.user_opt = of_user.current_opt;
		ls.session_opt = of_session.current_opt;
		write_launch_state(ls);

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

void clean_line(struct uint_point origin, uint line) {
  // - outline + nullbyte
  static char line_cleaner[BOX_WIDTH - 2 + 1] = {
      [0 ... BOX_WIDTH - 2 - 1] = ' ', [BOX_WIDTH - 2] = '\0'};
  printf("\x1b[%d;%dH%s", origin.y + line, origin.x + 1, line_cleaner);
}

u_char get_render_pos_offset(struct opts_field* self, u_char maxlen) {
  if (self->current_opt != 0) return 0;

  u_char pos = utf8len_until(self->efield.content,
                             &self->efield.content[self->efield.pos]);
  return pos - ofield_display_cursor_col(self, maxlen);
}

void print_session(struct uint_point origin, struct session session,
                   bool multiple) {
  clean_line(origin, SESSION_ROW);

  const char* NNULLABLE session_type;
  if (session.type == XORG) {
    session_type = g_config->strings.s_xorg;
  } else if (session.type == WAYLAND) {
    session_type = g_config->strings.s_wayland;
  } else if (session.type == SHELL) {
    session_type = g_config->strings.s_shell;
  } else {
    __builtin_unreachable();
  }

  // already in the box, - 1 bcs no need to step over margin, same reasoning in
  // other places
  printf(
      "\r\x1b[%luC\x1b[%sm%s\x1b[%sm",
      (ulong)(origin.x + VALUES_COL - VALUES_SEPR - utf8len(session_type) - 1),
      g_config->colors.e_header, session_type, g_config->colors.fg);

  char* session_color;
  if (session.type == XORG) {
    session_color = g_config->colors.s_xorg;
  } else if (session.type == WAYLAND) {
    session_color = g_config->colors.s_wayland;
  } else {
    session_color = g_config->colors.s_shell;
  }

  char* toprint = session.name;
  if (multiple) {
    u_char maxlen = VALUE_MAXLEN - utf8len(g_config->strings.opts_pre) -
                    utf8len(g_config->strings.opts_post);
    toprint += get_render_pos_offset(&of_session, maxlen);
    size_t printlen = utf8seekn(toprint, maxlen) - toprint;

    printf("\r\x1b[%dC%s\x1b[%sm%.*s\x1b[%sm%s", origin.x + VALUES_COL - 1,
           g_config->strings.opts_pre, session_color, (int)printlen, toprint,
           g_config->colors.fg, g_config->strings.opts_post);
  } else {
    toprint += get_render_pos_offset(&of_session, VALUE_MAXLEN);
    size_t printlen = utf8seekn(toprint, VALUE_MAXLEN) - toprint;
    printf("\r\x1b[%dC\x1b[%sm%.*s\x1b[%sm", origin.x + VALUES_COL - 1,
           session_color, (int)printlen, toprint, g_config->colors.fg);
  }
}

void print_user(struct uint_point origin, struct user user, bool multiple) {
  clean_line(origin, USER_ROW);
  printf("\r\x1b[%luC\x1b[%sm%s\x1b[%sm",
         (ulong)(origin.x + VALUES_COL - VALUES_SEPR -
                 utf8len(g_config->strings.e_user) - 1),
         g_config->colors.e_header, g_config->strings.e_user,
         g_config->colors.fg);

  char* user_color = g_config->colors.e_user;

  char* toprint = user.display_name;
  if (multiple) {
    u_char maxlen = VALUE_MAXLEN - utf8len(g_config->strings.opts_pre) -
                    utf8len(g_config->strings.opts_post);
    toprint += get_render_pos_offset(&of_session, maxlen);
    size_t printlen = utf8seekn(toprint, maxlen) - toprint;

    printf("\r\x1b[%dC< \x1b[%sm%.*s\x1b[%sm >", origin.x + VALUES_COL - 1,
           user_color, (int)printlen, toprint, g_config->colors.fg);
  } else {
    toprint += get_render_pos_offset(&of_user, VALUE_MAXLEN);
    size_t printlen = utf8seekn(toprint, VALUE_MAXLEN) - toprint;
    printf("\r\x1b[%dC\x1b[%sm%.*s\x1b[%sm", origin.x + VALUES_COL - 1,
           user_color, (int)printlen, toprint, g_config->colors.fg);
  }
}

void print_passwd(struct uint_point origin, uint length, bool err) {
  char passwd_prompt[VALUE_MAXLEN + 1];
  clean_line(origin, PASSWD_ROW);
  printf("\r\x1b[%luC\x1b[%sm%s\x1b[%sm",
         (ulong)(origin.x + VALUES_COL - VALUES_SEPR -
                 utf8len(g_config->strings.e_passwd) - 1),
         g_config->colors.e_header, g_config->strings.e_passwd,
         g_config->colors.fg);

  char* pass_color;
  if (err)
    pass_color = g_config->colors.e_badpasswd;
  else
    pass_color = g_config->colors.e_passwd;

  ulong actual_len = length > VALUE_MAXLEN ? VALUE_MAXLEN : length;
  memset(passwd_prompt, ' ', VALUE_MAXLEN);
  memset(passwd_prompt, '*', actual_len);
  passwd_prompt[VALUE_MAXLEN] = '\0';

  printf("\r\x1b[%dC\x1b[%sm", origin.x + VALUES_COL - 1, pass_color);
  printf("%s", passwd_prompt);

  printf("\x1b[%sm", g_config->colors.fg);
}

static void print_empty_row(uint wid, uint n, char* edge1, char* edge2) {
  for (size_t i = 0; i < n; i++) {
    printf("%s\x1b[%dC%s\x1b[%dD\x1b[1B", edge1, wid, edge2, wid + 2);
  }
}

static void print_row(uint wid, uint n, char* edge1, char* edge2,
                      char* filler) {
  for (size_t i = 0; i < n; i++) {
    printf("%s", edge1);
    for (size_t i = 0; i < wid; i++) {
      printf("%s", filler);
    }
    printf("%s\x1b[%dD\x1b[1B", edge2, wid + 2);
  }
}

static void print_box() {
  const struct uint_point BSTART = box_start();

  printf("\x1b[%d;%dH\x1b[%sm", BSTART.y, BSTART.x, g_config->colors.e_box);
  print_row(BOX_WIDTH - 2, 1, g_config->chars.ctl, g_config->chars.ctr,
            g_config->chars.hb);
  print_empty_row(BOX_WIDTH - 2, BOX_HEIGHT - 2, g_config->chars.vb,
                  g_config->chars.vb);
  print_row(BOX_WIDTH - 2, 1, g_config->chars.cbl, g_config->chars.cbr,
            g_config->chars.hb);
  printf("\x1b[%sm", g_config->colors.fg);
  (void)fflush(stdout);
}

static void print_footer() {
  size_t bsize = utf8len(g_config->strings.f_poweroff) +
                 utf8len(KEY_NAMES[g_config->functions.poweroff]) +
                 utf8len(g_config->strings.f_reboot) +
                 utf8len(KEY_NAMES[g_config->functions.reboot]) +
                 utf8len(g_config->strings.f_refresh) +
                 utf8len(KEY_NAMES[g_config->functions.refresh]);

  bsize += 2 * 2 + // 2 wide separators between 3 items
           3 * 1;  // 3 thin separators inside every item

  uint row = window.ws_row - 1;
  uint col = window.ws_col - 2 - bsize;
  printf(
      "\x1b[%3$d;%4$dH%8$s \x1b[%1$sm%5$s\x1b[%2$sm  %9$s "
      "\x1b[%1$sm%6$s\x1b[%2$sm  %10$s \x1b[%1$sm%7$s\x1b[%2$sm",
      g_config->colors.e_key, g_config->colors.fg, row, col,
      KEY_NAMES[g_config->functions.poweroff],
      KEY_NAMES[g_config->functions.reboot],
      KEY_NAMES[g_config->functions.refresh], g_config->strings.f_poweroff,
      g_config->strings.f_reboot, g_config->strings.f_refresh);
  (void)fflush(stdout);
}

void print_err(const char* msg) {
  struct uint_point origin = box_start();
  (void)fprintf(stderr, "\x1b[%d;%dH%s(%d): %s", origin.y - 1, origin.x, msg,
                errno, strerror(errno));
}

void print_errno(const char* descr) {
  struct uint_point origin = box_start();
  if (descr == NULL)
    (void)fprintf(stderr, "\x1b[%d;%dH\x1b[%smunknown error(%d): %s",
                  origin.y - 1, origin.x, g_config->colors.err, errno,
                  strerror(errno));
  else {
    (void)fprintf(stderr, "\x1b[%d;%dH\x1b[%sm%s(%d): %s", origin.y - 1,
                  origin.x, g_config->colors.err, descr, errno,
                  strerror(errno));
  }
}

void restore_all() {
  // restore cursor pos, restore screen and show cursor
  (void)printf("\x1b[u\x1b[?47l\x1b[?25h");
  (void)fflush(stdout);
  tcsetattr(STDOUT_FILENO, TCSANOW, &orig_term);
}

void signal_handler(int code) {
  restore_all();
  exit(code);
}
