#ifndef _LAUNCHSTATEH_
#define _LAUNCHSTATEH_

struct LaunchState {
	int user_opt;
	int session_opt;
};

struct LaunchState read_launch_state();
bool write_launch_state(struct LaunchState state);

#endif
