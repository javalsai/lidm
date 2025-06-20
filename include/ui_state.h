#ifndef UISTATEH_
#define UISTATEH_

#include <stddef.h>

#include "ui.h"

extern enum input focused_input;

extern struct opts_field of_session;
extern struct opts_field of_user;
extern struct opts_field of_passwd;

extern struct Vector* gusers;
extern struct Vector* gsessions;

struct opts_field* NNULLABLE get_opts_field(enum input from);
struct opts_field* NNULLABLE get_opts_ffield();

struct user st_user();
struct session st_session(bool include_defshell);

void st_ch_focus(char direction);
void st_ch_of_opts(char direction);
void st_ch_ef_col(char direction);
void st_kbd_type(char* text, bool cfg_include_defshell);

#endif
