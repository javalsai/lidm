// Small file to save last selection

#define STATE_PATH "/var/lib/lidm/state"

#include <stdio.h>
#include <stdbool.h>

#include "launch_state.h"

struct LaunchState read_launch_state() {
	struct LaunchState state;
	state.user_opt = 1;
	state.session_opt = 1;
	FILE* f = fopen(STATE_PATH, "r");
	if(!f) return state;
	fscanf(f, "%i;%i", &state.user_opt, &state.session_opt);
	fclose(f);
	return state;
}

bool write_launch_state(struct LaunchState state) {
	FILE* f = fopen(STATE_PATH, "w");
	if(!f) return false;
	fprintf(f, "%i;%i", state.user_opt, state.session_opt);
	fclose(f);
	return true;
}
