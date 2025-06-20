#ifndef UIH_
#define UIH_

#include "config.h"
#include "ofield.h"
#include "util.h"

enum input { SESSION, USER, PASSWD };
extern const u_char inputs_n;

// not customizable (for now)
extern const uint boxw;
extern const uint boxh;

void setup(struct config* config);
int load(struct Vector* users, struct Vector* sessions);
void print_err(const char*);
void print_errno(const char*);

void ui_update_field(enum input focused_input);
void ui_update_ffield();
void ui_update_ofield(struct opts_field* self);
void ui_update_cursor_focus();

#endif
