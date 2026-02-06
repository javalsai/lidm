#ifndef UIH_
#define UIH_

#include "config.h"
#include "ofield.h"
#include "util/vec.h"

//    [box_start]
//    ↓
// 0 [┌]───────────────────────────────────────────────┐
// 1  │                                                │
// 2  │    the-art                2025-06-20 21:40:44  │
// 3  │                                             |  │
// 4  │                                             |  │
// 5  │       xorg   < Default >                    |  │
// 6  │                                             |  │
// 7  │       user   javalsai                       |  │
// 8  │                                             |  │
// 9  │   password   ________________________________  │
// 10 │          |                                  |  │
// 11 └────────────────────────────────────────────────┘
//    01234567890123456789012345678901234567890123456789
//    00000000001111111111222222222233333333334444444444
//    |--|       |   ↑                              |--|[BOX_HMARGIN]
//    [BOX_HMARGIN]  [VALUES_COL]                    |
//               |---|[VALUES_SEPR]                  |
//                  |--------------------------------|[VALUE_MAXLEN]

#define HEAD_ROW 2
#define SESSION_ROW 5
#define USER_ROW 7
#define PASSWD_ROW 9

#define BOX_WIDTH 50
#define BOX_HEIGHT 12
#define BOX_HMARGIN 2

#define VALUES_COL 15
#define VALUES_SEPR 3
#define VALUE_MAXLEN (BOX_WIDTH - VALUES_COL + 1 - BOX_HMARGIN - 2)

enum Input { SESSION, USER, PASSWD };
extern const u_char INPUTS_N;

void setup(struct config* config);
int load(struct Vector* users, struct Vector* sessions);
void print_err(const char* /*msg*/);
void print_errno(const char* /*descr*/);
void print_pam_msg(const char* msg, int msg_style);
void clear_pam_msg(void);

void ui_update_field(enum Input focused_input);
void ui_update_ffield();
void ui_update_ofield(struct opts_field* self);
void ui_update_cursor_focus();

#endif
