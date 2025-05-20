// Small file to save last selection

#define STATE_PATH "/var/lib/lidm/state"

#include <stdio.h>
#include <stdbool.h>

#include "launch_state.h"

static void serialize_lstate(FILE* f, struct LaunchState st) {
	fprintf(f, "%i;%i", st.user_opt, st.session_opt);
}

static void deserialize_lstate(FILE* f, struct LaunchState* st) {
	fscanf(f, "%i;%i", &st->user_opt, &st->session_opt);
}

struct LaunchState read_launch_state() {
	struct LaunchState state;
	state.user_opt = 1;
	state.session_opt = 1;
	FILE* f = fopen(STATE_PATH, "r");
	if(!f) return state;
	deserialize_lstate(f, &state);
	fclose(f);
	return state;
}

bool write_launch_state(struct LaunchState state) {
	FILE* f = fopen(STATE_PATH, "w");
	if(!f) return false;
	serialize_lstate(f, state);
	fclose(f);
	return true;
}
