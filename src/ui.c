// i'm sorry
// really sorry

#include <asm-generic/errno.h>
#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <security/pam_appl.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
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
#include "launch_state.h"
#include "log.h"
#include "macros.h"
#include "ofield.h"
#include "sessions.h"
#include "ui.h"
#include "ui_state.h"
#include "users.h"
#include "util.h"

const u_char INPUTS_N = 3;

struct uint_point {
  uint x;
  uint y;
};

static void print_box();
static void print_head();
static void print_footer();
static void restore_all();
static void signal_handler(int code);

static void print_session(struct session session, bool multiple);
static void print_user(struct user user, bool multiple);
static void print_passwd(uint length, bool err);
static void scratch_print_ui();

// ansi resource: https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
static struct termios orig_term;
static struct termios term;
static struct winsize window;

#define INNER_BOX_OUT_MARGIN 2
struct config* g_config = NULL;

static volatile sig_atomic_t need_resize = 0;

static void process_sigwinch(int signal) {
  UNUSED(signal);
  need_resize = 1;
}

static inline void draw_bg() {
  // apply bg color to all screen
  printf("\x1b[%sm\x1b[2J", g_config->colors.bg);
}

void setup(struct config* config) {
  g_config = config;

  tcgetattr(STDOUT_FILENO, &orig_term);
  term = orig_term; // save term
  // "stty" attrs
  term.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDOUT_FILENO, TCSANOW, &term);

  // save cursor pos, save screen
  printf("\x1b[s\x1b[?47h");
  draw_bg();

  (void)atexit(restore_all);
  (void)signal(SIGINT, signal_handler);
  (void)signal(SIGWINCH, process_sigwinch);
  (void)fflush(stdout);
}

static struct uint_point box_start = {.x = 0, .y = 0};

#define STRFTIME_PREALLOC 64
#define TM_YEAR_EPOCH 1900
static char* fmt_time(const char* fmt) {
  time_t tme = time(NULL);
  struct tm tm = *localtime(&tme);

  size_t alloc_size = STRFTIME_PREALLOC;
  char* buf = malloc(alloc_size);
  if (!buf) return NULL;
  while (true) {
    if (strftime(buf, alloc_size, fmt, &tm) != 0 && strlen(fmt) != 0)
      return buf;

    alloc_size *= 2;
    char* nbuf = realloc(buf, alloc_size);
    if (!nbuf) {
      free(buf);
      return NULL;
    }
    buf = nbuf;
  }
}

char* trunc_gethostname(size_t maxlen, const char* const ELLIPSIS) {
  if (utf8len(ELLIPSIS) > maxlen) return NULL;
  size_t alloc_size = HOST_NAME_MAX + strlen(ELLIPSIS) + 1;
  char* buf = malloc(alloc_size);
  if (!buf) return NULL;

  if (gethostname(buf, alloc_size) != 0) {
    free(buf);
    return NULL;
  }

  if (utf8len(buf) > maxlen) {
    size_t end = utf8trunc(buf, maxlen - utf8len(ELLIPSIS));
    strcpy(&buf[end], ELLIPSIS);
  }
  return buf;
}

void ui_update_cursor_focus() {
  u_char line = box_start.y;
  u_char col = box_start.x + VALUES_COL;

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
}

void ui_update_field(enum Input focused_input) {
  if (focused_input == PASSWD) {
    print_passwd(utf8len(of_passwd.efield.content), false);
  } else if (focused_input == SESSION) {
    print_session(st_session(g_config->behavior.include_defshell),
                  of_session.opts > 1);
  } else if (focused_input == USER) {
    print_user(st_user(), of_user.opts > 1);
    ui_update_field(SESSION);
  }

  ui_update_cursor_focus();
}

void ui_update_ffield() {
  ui_update_field(focused_input);
}

void ui_update_ofield(struct opts_field* NNULLABLE self) {
  enum Input input;
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

/// draw everything
void scratch_print_ui() {
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &window);
  box_start = (struct uint_point){
      .x = ((window.ws_col - BOX_WIDTH) / 2) + 1, // looks better
      .y = ((window.ws_row - BOX_HEIGHT) / 2),    // leave more space under
  };

  if (window.ws_row < BOX_HEIGHT + (INNER_BOX_OUT_MARGIN * 2) ||
      window.ws_col < BOX_WIDTH + (INNER_BOX_OUT_MARGIN * 2)) {
    printf("\033[2J\033[H"); // Clear screen
    printf("\x1b[1;31mScreen too small\x1b[0m\n");
    printf("\x1b[%s;%sm\x1b[2J", g_config->colors.bg, g_config->colors.fg);
    return;
  }

  printf("\033[2J\033[H\033c"); // Clear screen
  draw_bg();

  /// PRINTING
  // printf box
  print_box();

  print_head();
  print_footer();

  ui_update_field(SESSION);
  ui_update_field(USER);
  ui_update_field(PASSWD);
  ui_update_cursor_focus();
}

#define MS_PER_S 1000
#define US_PER_MS 1000
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
int load(struct Vector* users, struct Vector* sessions) {
  long long ms_time = g_config->behavior.refresh_rate;
  ms_time = ms_time < 0 ? 0 : ms_time;

  /// SETUP
  gusers = users;
  gsessions = sessions;

  of_session =
      ofield_new(sessions->length + g_config->behavior.include_defshell);
  of_user = ofield_new(users->length);
  of_passwd = ofield_new(0);

  of_user.current_opt = users->length > 0;
  of_session.current_opt = sessions->length > 0;
  struct LaunchState initial_state;
  if (read_launch_state(&initial_state) == 0) {
    for (size_t i = 0; i < users->length; i++) {
      struct user* user_i = (struct user*)vec_get(users, i);
      if (strcmp(user_i->username, initial_state.username) == 0) {
        of_user.current_opt = i + 1;
        break;
      }
    }

    for (size_t i = 0; i < sessions->length; i++) {
      struct session* session_i = (struct session*)vec_get(sessions, i);
      if (strcmp(session_i->name, initial_state.session_opt) == 0) {
        of_session.current_opt = i + 1;
        break;
      }
    }
    if (g_config->behavior.include_defshell) {
      if (strcmp(st_user().shell, initial_state.session_opt) == 0)
        of_session.current_opt = sessions->length + 1;
    }

    free(initial_state.username);
    free(initial_state.session_opt);
  }

  scratch_print_ui();

  /// INTERACTIVE
  u_char len;
  char seq[0xff];
  uint esc = 0;
  while (true) {
    if (need_resize) {
      need_resize = 0;
      scratch_print_ui();
    } else {
      // partial refresh
      print_head();
      ui_update_cursor_focus();
    }

    struct timeval tv;
    tv.tv_usec = (ms_time % MS_PER_S) * US_PER_MS;
    tv.tv_sec = ms_time / MS_PER_S;

    (void)fflush(stdout);
    if (!read_press_nb(&len, seq, &tv)) continue;
    if (*seq == '\x1b') {
      struct option_keys ansi_code = find_ansi(seq);
      if (ansi_code.is_some) {
        enum Keys ansi_key = ansi_code.key;
        if (ansi_key == ESC) {
          esc = 2;
        } else if (ansi_key == g_config->functions.refresh) {
          restore_all();
          return 0;
        } else if (ansi_key == g_config->functions.reboot) {
          restore_all();
          reboot(RB_AUTOBOOT);
          exit(0);
        } else if (ansi_key == g_config->functions.poweroff) {
          restore_all();
          reboot(RB_POWER_OFF);
          exit(0);
        } else if (g_config->functions.fido != NONE &&
                   ansi_key == g_config->functions.fido) {
          bool successful_write = write_launch_state((struct LaunchState){
              .username = st_user().username,
              .session_opt =
                  st_session(g_config->behavior.include_defshell).name,
          });
          if (!successful_write) log_puts("[E] failed to write launch state");

          if (!launch(st_user().username, "",
                      st_session(g_config->behavior.include_defshell),
                      &restore_all, g_config)) {
            print_passwd(utf8len(of_passwd.efield.content), true);
            ui_update_cursor_focus();
          } else {
            scratch_print_ui();
          }
        } else if (ansi_key == A_UP || ansi_key == A_DOWN) {
          st_ch_focus(ansi_key == A_DOWN ? 1 : -1);
        } else if (ansi_key == A_RIGHT || ansi_key == A_LEFT) {
          if (esc)
            st_ch_of_opts(ansi_key == A_RIGHT ? 1 : -1);
          else
            st_ch_ef_col(ansi_key == A_RIGHT ? 1 : -1);
        }
      }
    } else {
      if (len == 1 && *seq == '\n') {
        bool successful_write = write_launch_state((struct LaunchState){
            .username = st_user().username,
            .session_opt = st_session(g_config->behavior.include_defshell).name,
        });
        if (!successful_write) log_puts("[E] failed to write launch state");

        if (!launch(st_user().username, of_passwd.efield.content,
                    st_session(g_config->behavior.include_defshell),
                    &restore_all, g_config)) {
          print_passwd(utf8len(of_passwd.efield.content), true);
          ui_update_cursor_focus();
        } else {
          scratch_print_ui();
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

void print_head() {
  char* fmtd_time = fmt_time(g_config->behavior.timefmt);
  size_t len_tm = utf8len(fmtd_time);

  // calculate the space available for the host name
  ssize_t hostname_size = BOX_WIDTH - (BOX_HMARGIN * 2) - len_tm - VALUES_SEPR;
  if (hostname_size < 0) hostname_size = 0;

  // hostname doesn't just change on runtime,
  // but the length of the time string might
  static char* NULLABLE hostname = NULL;
  static ssize_t hostname_calcd_size;

  // save the truncated hostname and the length it truncated to,
  // if said length changes recalculate this (and free previous str)
  if (!hostname || hostname_calcd_size != hostname_size) {
    if (hostname) free(hostname);
    hostname = trunc_gethostname(hostname_size, g_config->strings.ellipsis);
    hostname_calcd_size = hostname_size;
  }

  clean_line(box_start, HEAD_ROW);

  // put hostname
  if (hostname_size)
    printf("\x1b[%dG\x1b[%sm%s\x1b[%sm", box_start.x + 1 + BOX_HMARGIN,
           g_config->colors.e_hostname, hostname ? hostname : "unknown",
           g_config->colors.fg);

  // put date
  printf("\x1b[%dG\x1b[%sm%s\x1b[%sm",
         box_start.x + BOX_WIDTH - 1 - BOX_HMARGIN - (uint)len_tm,
         g_config->colors.e_date, fmtd_time, g_config->colors.fg);

  free(fmtd_time);
}

void print_session(struct session session, bool multiple) {
  clean_line(box_start, SESSION_ROW);

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
  printf("\r\x1b[%luC\x1b[%sm%s\x1b[%sm",
         (ulong)(box_start.x + VALUES_COL - VALUES_SEPR -
                 utf8len(session_type) - 1),
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

    printf("\r\x1b[%dC%s\x1b[%sm%.*s\x1b[%sm%s", box_start.x + VALUES_COL - 1,
           g_config->strings.opts_pre, session_color, (int)printlen, toprint,
           g_config->colors.fg, g_config->strings.opts_post);
  } else {
    toprint += get_render_pos_offset(&of_session, VALUE_MAXLEN);
    size_t printlen = utf8seekn(toprint, VALUE_MAXLEN) - toprint;
    printf("\r\x1b[%dC\x1b[%sm%.*s\x1b[%sm", box_start.x + VALUES_COL - 1,
           session_color, (int)printlen, toprint, g_config->colors.fg);
  }
}

void print_user(struct user user, bool multiple) {
  clean_line(box_start, USER_ROW);
  printf("\r\x1b[%luC\x1b[%sm%s\x1b[%sm",
         (ulong)(box_start.x + VALUES_COL - VALUES_SEPR -
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

    printf("\r\x1b[%dC< \x1b[%sm%.*s\x1b[%sm >", box_start.x + VALUES_COL - 1,
           user_color, (int)printlen, toprint, g_config->colors.fg);
  } else {
    toprint += get_render_pos_offset(&of_user, VALUE_MAXLEN);
    size_t printlen = utf8seekn(toprint, VALUE_MAXLEN) - toprint;
    printf("\r\x1b[%dC\x1b[%sm%.*s\x1b[%sm", box_start.x + VALUES_COL - 1,
           user_color, (int)printlen, toprint, g_config->colors.fg);
  }
}

void print_passwd(uint length, bool err) {
  char passwd_prompt[VALUE_MAXLEN + 1];
  clean_line(box_start, PASSWD_ROW);
  printf("\r\x1b[%luC\x1b[%sm%s\x1b[%sm",
         (ulong)(box_start.x + VALUES_COL - VALUES_SEPR -
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

  printf("\r\x1b[%dC\x1b[%sm", box_start.x + VALUES_COL - 1, pass_color);
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
  printf("\x1b[%d;%dH\x1b[%sm", box_start.y, box_start.x,
         g_config->colors.e_box);
  print_row(BOX_WIDTH - 2, 1, g_config->chars.ctl, g_config->chars.ctr,
            g_config->chars.hb);
  print_empty_row(BOX_WIDTH - 2, BOX_HEIGHT - 2, g_config->chars.vb,
                  g_config->chars.vb);
  print_row(BOX_WIDTH - 2, 1, g_config->chars.cbl, g_config->chars.cbr,
            g_config->chars.hb);
  printf("\x1b[%sm", g_config->colors.fg);
}

static void print_footer() {
  bool fido_enabled = g_config->functions.fido != NONE;

  size_t bsize = utf8len(g_config->strings.f_poweroff) +
                 utf8len(KEY_NAMES[g_config->functions.poweroff]) +
                 utf8len(g_config->strings.f_reboot) +
                 utf8len(KEY_NAMES[g_config->functions.reboot]) +
                 utf8len(g_config->strings.f_refresh) +
                 utf8len(KEY_NAMES[g_config->functions.refresh]);

  bsize += (2 * 2) + (3 * 1);

  if (fido_enabled) {
    bsize += utf8len(g_config->strings.f_fido) +
             utf8len(KEY_NAMES[g_config->functions.fido]) + 2 + 1;
  }

  uint row = window.ws_row - 1;
  uint col = window.ws_col - 2 - bsize;

  printf("\x1b[%d;%dH%s \x1b[%sm%s\x1b[%sm  %s \x1b[%sm%s\x1b[%sm  ", row, col,
         g_config->strings.f_poweroff, g_config->colors.e_key,
         KEY_NAMES[g_config->functions.poweroff], g_config->colors.fg,
         g_config->strings.f_reboot, g_config->colors.e_key,
         KEY_NAMES[g_config->functions.reboot], g_config->colors.fg);

  if (fido_enabled) {
    printf("%s \x1b[%sm%s\x1b[%sm  ", g_config->strings.f_fido,
           g_config->colors.e_key, KEY_NAMES[g_config->functions.fido],
           g_config->colors.fg);
  }

  printf("%s \x1b[%sm%s\x1b[%sm", g_config->strings.f_refresh,
         g_config->colors.e_key, KEY_NAMES[g_config->functions.refresh],
         g_config->colors.fg);
}

void print_err(const char* msg) {
  (void)fprintf(stderr, "\x1b[%d;%dH%s(%d): %s", box_start.y - 1, box_start.x,
                msg, errno, strerror(errno));
}

void print_pam_msg(const char* msg, int msg_style) {
  uint row = box_start.y + BOX_HEIGHT + 1;
  const char* color =
      (msg_style == PAM_ERROR_MSG) ? g_config->colors.err : g_config->colors.fg;
  printf("\x1b[%d;%dH\x1b[K\x1b[%sm%.*s\x1b[%sm", row, box_start.x, color,
         BOX_WIDTH, msg, g_config->colors.fg);
  (void)fflush(stdout);
}

void clear_pam_msg(void) {
  uint row = box_start.y + BOX_HEIGHT + 1;
  printf("\x1b[%d;%dH\x1b[K", row, box_start.x);
  (void)fflush(stdout);
}

void print_errno(const char* descr) {
  if (descr == NULL)
    (void)fprintf(stderr, "\x1b[%d;%dH\x1b[%smunknown error(%d): %s",
                  box_start.y - 1, box_start.x, g_config->colors.err, errno,
                  strerror(errno));
  else {
    (void)fprintf(stderr, "\x1b[%d;%dH\x1b[%sm%s(%d): %s", box_start.y - 1,
                  box_start.x, g_config->colors.err, descr, errno,
                  strerror(errno));
  }
}

void restore_all() {
  // restore cursor pos, restore screen and show cursor
  (void)printf("\x1b[u\x1b[?47l\x1b[?25h");
  tcsetattr(STDOUT_FILENO, TCSANOW, &orig_term);
}

void signal_handler(int code) {
  restore_all();
  exit(code);
}
