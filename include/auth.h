#ifndef AUTHH_
#define AUTHH_

#include <stdbool.h>

#include "config.h"
#include "sessions.h"

bool launch(char* NNULLABLE user, char* NNULLABLE passwd,
            struct session session, void (*NULLABLE cb)(void),
            struct config* NNULLABLE config);

#endif
