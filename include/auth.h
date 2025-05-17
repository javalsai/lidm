#ifndef _AUTHH_
#define _AUTHH_

#include <stdbool.h>

#include "config.h"
#include "sessions.h"

bool launch(char *user, char *passwd, struct session session, void (*cb)(void),
            struct behavior *behavior);

#endif
