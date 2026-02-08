// Small file for saving last selection

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "launch_state.h"

#define STATE_DIR "/var/lib/lidm"
#define STATE_FILE "/var/lib/lidm/state"

#define RWXR_X___ 0750

int read_launch_state(struct LaunchState* NNULLABLE state) {
  FILE* state_fd = fopen(STATE_FILE, "r");
  if (state_fd == NULL) return -1;

  *state = (struct LaunchState){
      .username = NULL,
      .session_opt = NULL,
  };

  size_t num = 0;
  ssize_t chars = getline(&state->username, &num, state_fd);
  if (chars <= 0) goto fail;
  if (state->username[chars - 1] == '\n') state->username[chars - 1] = 0;

  num = 0;
  chars = getline(&state->session_opt, &num, state_fd);
  if (chars <= 0) {
    free(state->session_opt);
    goto fail;
  }
  if (state->session_opt[chars - 1] == '\n') state->session_opt[chars - 1] = 0;

  (void)fclose(state_fd);
  return 0;

fail:
  (void)fclose(state_fd);
  return -1;
}

bool write_launch_state(struct LaunchState state) {
  FILE* state_fd = fopen(STATE_FILE, "w");
  if (state_fd == NULL) {
    if (mkdir(STATE_DIR, RWXR_X___) == -1) return false;
    state_fd = fopen(STATE_FILE, "w");
    if (state_fd == NULL) return false;
  }
  (void)fprintf(state_fd, "%s\n%s\n", state.username, state.session_opt);
  (void)fclose(state_fd);
  return true;
}
