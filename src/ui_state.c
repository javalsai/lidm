#include "ui_state.h"
#include "ofield.h"
#include "sessions.h"
#include "ui.h"
#include "users.h"

enum input focused_input = PASSWD;

struct Vector* gusers;
struct Vector* gsessions;

struct opts_field of_session;
struct opts_field of_user;
struct opts_field of_passwd;

struct opts_field* NNULLABLE get_opts_field(enum input from) {
  if (from == SESSION) return &of_session;
  if (from == USER) return &of_user;
  if (from == PASSWD) return &of_passwd;
  __builtin_unreachable();
}

struct opts_field* NNULLABLE get_opts_ffield() {
  return get_opts_field(focused_input);
}

struct user st_user() {
  if (of_user.current_opt != 0)
    return *(struct user*)vec_get(gusers, of_user.current_opt - 1);

  struct user custom_user;
  custom_user.shell = "/usr/bin/bash";
  custom_user.username = custom_user.display_name = of_user.efield.content;
  return custom_user;
}
struct session st_session(bool include_defshell) {
  if (of_session.current_opt != 0) {
    // this is for the default user shell :P, not the greatest
    // implementation but I want to get his done
    if (include_defshell && of_session.current_opt == gsessions->length + 1) {
      struct session shell_session;
      shell_session.type = SHELL;
      shell_session.exec = shell_session.name = st_user().shell;
      return shell_session;
    }

    return *(struct session*)vec_get(gsessions, of_session.current_opt - 1);
  }

  struct session custom_session;
  custom_session.type = SHELL;
  custom_session.name = custom_session.exec = of_session.efield.content;
  return custom_session;
}

void st_ch_focus(char direction) {
  focused_input = (focused_input + direction) % inputs_n;
  ui_update_cursor_focus();
}

void st_ch_of_opts(char direction) {
  struct opts_field* ffield = get_opts_ffield();
  if (focused_input == PASSWD) ffield = &of_session;
  if (!ofield_opts_seek(ffield, direction)) {
    if (focused_input == PASSWD || focused_input == SESSION)
      ofield_opts_seek(&of_user, direction);
    else
      ofield_opts_seek(&of_session, direction);
  }
}

void st_ch_ef_col(char direction) {
  struct opts_field* ffield = get_opts_ffield();
  if (!ofield_seek(ffield, direction))
    if (!ofield_opts_seek(&of_session, direction))
      ofield_opts_seek(&of_user, direction);

  ui_update_cursor_focus();
}

void st_kbd_type(char* text, bool cfg_include_defshell) {
  struct opts_field* field = get_opts_ffield();
  char* start = "";
  if (focused_input == USER && of_user.current_opt != 0)
    start = st_user().username;
  if (focused_input == SESSION && of_session.current_opt != 0 &&
      st_session(cfg_include_defshell).type == SHELL)
    start = st_session(cfg_include_defshell).exec;

  ofield_kbd_type(field, text, start);
  ui_update_ffield();
}
