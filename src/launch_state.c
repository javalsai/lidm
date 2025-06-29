// Small file for saving last selection

#define STATE_DIR "/var/lib/lidm"
#define STATE_FILE "/var/lib/lidm/state"

#include "launch_state.h"

struct LaunchState read_launch_state() {
  struct LaunchState state;
  state.user_opt = 1;
  state.session_opt = 1;
  FILE* state_fd = fopen(STATE_FILE, "r");
  if (state_fd == NULL) return state;
  if (fscanf(state_fd, "%i;%i", &state.user_opt, &state.session_opt) != 2) {
    state.user_opt = 1;
    state.session_opt = 1;
  }
  (void)fclose(state_fd);
  return state;
}

bool write_launch_state(struct LaunchState state) {
  FILE* state_fd = fopen(STATE_FILE, "w");
  if (state_fd == NULL) {
    if (mkdir(STATE_DIR, 0755) == -1) return false;
    state_fd = fopen(STATE_FILE, "w");
    if (state_fd == NULL) return false;
  }
  (void)fprintf(state_fd, "%i;%i", state.user_opt, state.session_opt);
  (void)fclose(state_fd);
  return true;
}
