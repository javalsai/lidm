// i'm sorry

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include <efield.h>
#include <keys.h>
#include <sessions.h>
#include <ui.h>
#include <util.h>

static void print_box();
static void print_footer();
static void restore_all();
static void signal_handler(int);

const uint boxw = 50;
const uint boxh = 12;

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

static struct theme theme;
static struct functions functions;
static struct strings strings;
static struct behavior behavior;
void setup(struct config __config) {
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &window);

  // 2 padding top and bottom for footer and vertical compensation
  // 2 padding left & right to not overflow footer width
  if (window.ws_row < boxh + 4 || window.ws_col < boxw + 4) {
    fprintf(stderr, "\x1b[1;31mScreen too small\x1b[0m\n");
    exit(1);
  }

  theme = __config.theme;
  functions = __config.functions;
  strings = __config.strings;

  tcgetattr(STDOUT_FILENO, &orig_term);
  term = orig_term; // save term
  // "stty" attrs
  term.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDOUT_FILENO, TCSANOW, &term);

  // save cursor pos, save screen, hide cursor, set color and reset screen
  // (applying color to all screen)
  printf("\x1b[s\x1b[?47h\x1b[%s;%sm\x1b[2J", theme.colors.bg,
         theme.colors.fg);

  print_footer();
  atexit(restore_all);
  signal(SIGINT, signal_handler);
}

static struct uint_point box_start() {
  struct uint_point __start;
  __start.x = (window.ws_col - boxw) / 2 + 1;
  __start.y = (window.ws_row - boxh) / 2 + 1;
  return __start;
}

static char *fmt_time() {
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  size_t bsize =
      snprintf(NULL, 0, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900,
               tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec) +
      1;
  char *buf = malloc(bsize);
  snprintf(buf, bsize, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900,
           tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  return buf;
}

// TODO: handle buffers longer than the buffer (cut str to the end, change
// cursor pos...) should just overlap for now

// ugh, this represent a field which might have options
// opts is the ammount of other options possible (0 will behave as a passwd)
// aaaand (it's an abstract idea, letme think), also holds the status of a
// custom content, like custom launch command or user or smth
struct opt_field {
  uint opts;
  uint current_opt; // 0 is edit mode btw
  struct editable_field efield;
};
static struct opt_field ofield_new(uint opts) {
  struct opt_field __field;
  __field.opts = opts;
  __field.current_opt = 1;
  if (opts == 0) {
    __field.current_opt = 0;
    __field.efield = field_new("");
  }
  return __field;
}
static void ofield_toedit(struct opt_field *ofield, char *init) {
  ofield->efield = field_new(init);
}
// true in case it was able to "use" the seek (a empty only editable field
// wouldn't)
static bool ofield_seek(struct opt_field *ofield, char seek) {
  if (ofield->current_opt == 0) {
    if (field_seek(&ofield->efield, seek)) {
      return true;
    }
  }

  if (ofield->opts == 0)
    return false;

  ofield->current_opt = (ofield->current_opt + seek + ofield->opts + 1) %
                        (ofield->opts + 1); // not sure about this one
  return true;
}
static u_char ofield_max_displ_pos(struct opt_field *ofield) {
  // TODO: set max cursor pos too
  // keep in mind that also have to keep in mind scrolling and ughhh, mentally blocked, but this is complex
  if(ofield->current_opt == 0)
    return ofield->efield.pos;
  else
    return 0;
}

enum input { SESSION, USER, PASSWD };
static u_char inputs_n = 3;
enum input focused_input = PASSWD;
struct opt_field of_session;
struct opt_field of_user;
struct opt_field of_passwd;

void update_cursor_focus() {
  struct uint_point bstart = box_start();
  u_char line = bstart.y;
  u_char row = bstart.x + 15;
  if (focused_input == SESSION) {
    line += 5;
    row += ofield_max_displ_pos(&of_session);
  } else if (focused_input == USER) {
    line += 7;
    row += ofield_max_displ_pos(&of_user);
  } else if (focused_input == PASSWD) {
    line += 9;
    row += ofield_max_displ_pos(&of_passwd);
  }
  printf("\x1b[%d;%dH", line, row);
  fflush(stdout);
}

// true = forward, false = backward
void change_field_focus(bool direction) {
  if (direction)
    focused_input = (focused_input + 1 + inputs_n) % inputs_n;
  else
    focused_input = (focused_input - 1 + inputs_n) % inputs_n;
}

int load(struct users_list *users, struct sessions_list *sessions) {
  /// SETUP

  // hostnames larger won't render properly
  char *hostname = malloc(16);
  if (gethostname(hostname, 16) != 0) {
    free(hostname);
    hostname = "unknown";
  } else {
    hostname = realloc(hostname, strlen(hostname) + 1);
  }

  of_session = ofield_new(sessions->length + behavior.include_defshell);
  of_user = ofield_new(users->length);
  of_passwd = ofield_new(0);

  /// PRINTING
  const struct uint_point boxstart = box_start();

  // printf box
  print_box();

  // put hostname
  printf("\x1b[%d;%dH\x1b[%sm%s\x1b[%sm", boxstart.y + 2,
         boxstart.x + 12 - (uint)strlen(hostname), theme.colors.e_hostname,
         hostname, theme.colors.fg);

  // put date
  char *fmtd_time = fmt_time();
  printf("\x1b[%d;%dH\x1b[%sm%s\x1b[%sm", boxstart.y + 2,
         boxstart.x + boxw - 3 - (uint)strlen(fmtd_time), theme.colors.e_date,
         fmtd_time, theme.colors.fg);

  print_session(boxstart, sessions->sessions[0], false);
  print_user(boxstart, users->users[0], false);
  print_passwd(boxstart, 5, false);
  fflush(stdout);
  update_cursor_focus();

  /// INTERACTIVE
  u_char len;
  char seq[256];

  while (true) {
    read_press(&len, seq);
    if (*seq == '\x1b') {
      enum keys ansi_code = find_ansi(seq);
      if (ansi_code != -1) {
        if (ansi_code == functions.refresh) {
          restore_all();
          return 0;
        } else if (ansi_code == functions.reboot) {
          system("reboot");
          exit(0);
        } else if (ansi_code == functions.poweroff) {
          system("poweroff");
          exit(0);
        } else if (ansi_code == A_UP || ansi_code == A_DOWN) {
          change_field_focus(ansi_code == A_UP);
        }
      }
    } else {}
      /*printf("norm(%d): %d\n", len, *seq);*/
  }
}

static char *line_cleaner = NULL;
static void clean_line(struct uint_point origin, uint line) {
  if (line_cleaner == NULL) {
    line_cleaner = malloc((boxw - 2) * sizeof(char));
    memset(line_cleaner, 32, boxw - 2);
  }
  printf("\x1b[%d;%dH", origin.y + line, origin.x + 1);
  fflush(stdout);
  write(STDOUT_FILENO, line_cleaner, boxw - 2);
}

// TODO: session_len > 32
static void print_session(struct uint_point origin, struct session session,
                          bool multiple) {
  clean_line(origin, 5);
  const char *session_type;
  if (session.type == XORG) {
    session_type = strings.s_xorg;
  } else if (session.type == WAYLAND) {
    session_type = strings.s_wayland;
  } else if (session.type == SHELL) {
    session_type = strings.s_shell;
  }
  printf("\r\x1b[%luC\x1b[%sm%s\x1b[%sm", origin.x + 11 - strlen(session_type),
         theme.colors.e_header, session_type, theme.colors.fg);

  char *session_color;
  if (session.type == XORG) {
    session_color = theme.colors.s_xorg;
  } else if (session.type == WAYLAND) {
    session_color = theme.colors.s_wl;
  } else if (session.type == SHELL) {
    session_color = theme.colors.s_shell;
  }

  if (multiple) {
    printf("\r\x1b[%dC< \x1b[%sm%s\x1b[%sm >", origin.x + 14, session_color,
           session.name, theme.colors.fg);
  } else {
    printf("\r\x1b[%dC\x1b[%sm%s\x1b[%sm", origin.x + 14, session_color,
           session.name, theme.colors.fg);
  }
}

// TODO: user_len > 32
static void print_user(struct uint_point origin, struct user user,
                       bool multiple) {
  clean_line(origin, 7);
  printf("\r\x1b[%luC\x1b[%sm%s\x1b[%sm",
         origin.x + 11 - strlen(strings.e_user), theme.colors.e_header,
         strings.e_user, theme.colors.fg);

  char *user_color = theme.colors.e_user;

  if (multiple) {
    printf("\r\x1b[%dC< \x1b[%sm%s\x1b[%sm >", origin.x + 14, user_color,
           user.display_name, theme.colors.fg);
  } else {
    printf("\r\x1b[%dC\x1b[%sm%s\x1b[%sm", origin.x + 14, user_color,
           user.display_name, theme.colors.fg);
  }
}

static char *passwd_prompt[32];
// TODO: passwd_len > 32
static void print_passwd(struct uint_point origin, uint length, bool err) {
  clean_line(origin, 9);
  printf("\r\x1b[%luC\x1b[%sm%s\x1b[%sm",
         origin.x + 11 - strlen(strings.e_passwd), theme.colors.e_header,
         strings.e_passwd, theme.colors.fg);

  char *pass_color;
  if (err)
    pass_color = theme.colors.e_badpasswd;
  else
    pass_color = theme.colors.e_passwd;

  memset(passwd_prompt, 32, 32);
  memset(passwd_prompt, '*', length);

  printf("\r\x1b[%dC\x1b[%sm\x1b[4m", origin.x + 14, pass_color);
  fflush(stdout);
  write(STDOUT_FILENO, passwd_prompt, 32);
  printf("\x1b[%sm", theme.colors.fg);
}

// ik this code is... *quirky*
// w just accounts for filler
// if filler == NULL, it will just move cursor
static void print_row(uint w, uint n, char *edge1, char *edge2, char **filler) {
  char *row;
  size_t row_size;

  uint size;
  if (filler == NULL) {
    row_size = snprintf(NULL, 0, "%s\x1b[%dC%s\x1b[%dD\x1b[1B", edge1, w, edge2,
                        w + 2) +
               1;
    row = malloc(row_size);
    snprintf(row, row_size, "%s\x1b[%dC%s\x1b[%dD\x1b[1B", edge1, w, edge2,
             w + 2);
  } else {
    size_t fillersize = strlen(*filler) * w;
    size_t nbytes1 = snprintf(NULL, 0, "%s", edge1) + 1;
    size_t nbytes2 = snprintf(NULL, 0, "%s\x1b[%dD\x1b[1B", edge2, w + 2) + 1;
    row_size = nbytes1 + fillersize + nbytes2;
    row = malloc(row_size);
    snprintf(row, nbytes1, "%s", edge1);
    for (uint i = 0; i < fillersize; i += strlen(*filler)) {
      strcpy(&row[nbytes1 + i], *filler);
    }
    snprintf(&row[nbytes1 + fillersize], nbytes2, "%s\x1b[%dD\x1b[1B", edge2,
             w + 2);
  }

  for (uint i = 0; i < n; i++) {
    write(STDOUT_FILENO, row, row_size);
  }
  free(row);
}

static void print_box() {
  // TODO: check min sizes
  const struct uint_point bstart = box_start();

  printf("\x1b[%d;%dH\x1b[%sm", bstart.y, bstart.x, theme.colors.e_box);
  fflush(stdout);
  print_row(boxw - 2, 1, theme.chars.ctl, theme.chars.ctr, &theme.chars.hb);
  print_row(boxw - 2, boxh - 2, theme.chars.vb, theme.chars.vb, NULL);
  print_row(boxw - 2, 1, theme.chars.cbl, theme.chars.cbr, &theme.chars.hb);
  printf("\x1b[%sm", theme.colors.fg);
  fflush(stdout);
}

static void print_footer() {
  size_t bsize = snprintf(NULL, 0, "%s %s  %s %s  %s %s", strings.f_poweroff,
                          key_names[functions.poweroff], strings.f_reboot,
                          key_names[functions.reboot], strings.f_refresh,
                          key_names[functions.refresh]);

  uint row = window.ws_row - 1;
  uint col = window.ws_col - 2 - bsize;
  printf("\x1b[%3$d;%4$dH%8$s \x1b[%1$sm%5$s\x1b[%2$sm  %9$s "
         "\x1b[%1$sm%6$s\x1b[%2$sm  %10$s \x1b[%1$sm%7$s\x1b[%2$sm",
         theme.colors.e_key, theme.colors.fg, row, col,
         key_names[functions.poweroff], key_names[functions.reboot],
         key_names[functions.refresh], strings.f_poweroff, strings.f_reboot,
         strings.f_refresh);
  fflush(stdout);
}

void print_err(const char *msg) {
  struct uint_point origin = box_start();
  fprintf(stderr, "\x1b[%d;%dH%s(%d): %s", origin.y - 1, origin.x, msg, errno,
          strerror(errno));
}

void print_errno(const char *descr) {
  struct uint_point origin = box_start();
  if (descr == NULL)
    fprintf(stderr, "\x1b[%d;%dH\x1b[%smunknown error(%d): %s", origin.y - 1,
            origin.x, theme.colors.err, errno, strerror(errno));
  else {
    fprintf(stderr, "\x1b[%d;%dH\x1b[%sm%s(%d): %s", origin.y - 1, origin.x,
            descr, theme.colors.err, errno, strerror(errno));
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
