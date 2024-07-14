#ifndef _AUTHH_
#define _AUTHH_

#include <stdbool.h>

#include <sessions.h>

bool check_passwd(char *user, char *passwd);
bool launch(char *user, char *passwd, struct session session, void (*cb)(void));

#endif
