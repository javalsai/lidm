#ifndef LAUNCHSTATEH_
#define LAUNCHSTATEH_

#include <stdbool.h>
#include <sys/stat.h>

#include "macros.h"

struct LaunchState {
  char* NNULLABLE username;
  char* NNULLABLE session_opt;
};

int read_launch_state(struct LaunchState* NNULLABLE state);
bool write_launch_state(struct LaunchState state);

#endif
