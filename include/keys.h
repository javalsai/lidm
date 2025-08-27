#ifndef KEYSH_
#define KEYSH_

#include <stdlib.h>

enum Keys {
  ESC,
  F1,
  F2,
  F3,
  F4,
  F5,
  F6,
  F7,
  F8,
  F9,
  F10,
  F11,
  F12,
  A_UP,
  A_DOWN,
  A_RIGHT,
  A_LEFT,
  N_CENTER,
  N_UP,
  N_DOWN,
  N_RIGHT,
  N_LEFT,
  INS,
  SUPR,
  HOME,
  END,
  PAGE_UP,
  PAGE_DOWN,
};

static const char* const KEY_NAMES[] = {
    [ESC] = "ESC",
    [F1] = "F1",
    [F2] = "F2",
    [F3] = "F3",
    [F4] = "F4",
    [F5] = "F5",
    [F6] = "F6",
    [F7] = "F7",
    [F8] = "F8",
    [F9] = "F9",
    [F10] = "F10",
    [F11] = "F11",
    [F12] = "F12",
    [A_UP] = "A_UP",
    [A_DOWN] = "A_DOWN",
    [A_RIGHT] = "A_RIGHT",
    [N_CENTER] = "N_CENTER",
    [A_LEFT] = "A_LEFT",
    [N_UP] = "N_UP",
    [N_DOWN] = "N_DOWN",
    [N_RIGHT] = "N_RIGHT",
    [N_LEFT] = "N_LEFT",
    [INS] = "INS",
    [SUPR] = "SUPR",
    [HOME] = "HOME",
    [END] = "END",
    [PAGE_UP] = "PAGE_UP",
    [PAGE_DOWN] = "PAGE_DOWN",
};

struct key_mapping {
  enum Keys key;
  const char* sequences[3];
};

static const struct key_mapping KEY_MAPPINGS[] = {
    {ESC, {"\x1b", NULL}},
    {F1, {"\x1bOP", "\x1b[[A", NULL}},
    {F2, {"\x1bOQ", "\x1b[[B", NULL}},
    {F3, {"\x1bOR", "\x1b[[C", NULL}},
    {F4, {"\x1bOS", "\x1b[[D", NULL}},
    {F5, {"\x1b[15~", "\x1b[[E", NULL}},
    {F6, {"\x1b[17~", NULL}},
    {F7, {"\x1b[18~", NULL}},
    {F8, {"\x1b[19~", NULL}},
    {F9, {"\x1b[20~", NULL}},
    {F10, {"\x1b[21~", NULL}},
    {F11, {"\x1b[23~", NULL}},
    {F12, {"\x1b[24~", NULL}},
    {A_UP, {"\x1b[A", NULL}},
    {A_DOWN, {"\x1b[B", NULL}},
    {A_RIGHT, {"\x1b[C", NULL}},
    {A_LEFT, {"\x1b[D", NULL}},
    {N_CENTER, {"\x1b[E", NULL}},
    {N_UP, {"\x1bOA", NULL}},
    {N_DOWN, {"\x1bOB", NULL}},
    {N_RIGHT, {"\x1bOC", NULL}},
    {N_LEFT, {"\x1bOD", NULL}},
    {INS, {"\x1b[2~", NULL}},
    {SUPR, {"\x1b[3~", NULL}},
    {HOME, {"\x1b[H", NULL}},
    {END, {"\x1b[F", NULL}},
    {PAGE_UP, {"\x1b[5~", NULL}},
    {PAGE_DOWN, {"\x1b[6~", NULL}},
};

#endif
