#ifndef PAM_H
#define PAM_H

#include <pwd.h>
#include <security/_pam_types.h>
#include <security/pam_appl.h>
#include <stdbool.h>

#include "macros.h"
#include "sessions.h"

#define PAMH_ERR_NOERR 0
#define PAMH_ERR_ALLOC 1
#define PAMH_ERR_ERRNO 2
#define PAMH_ERR_NOERRNO 3

struct pamh_getenv_status {
  char error_flag;
  union {
    char* NULLABLE* NNULLABLE envlist;
    const char* NNULLABLE errfn;
  };
};

// Doesn't include `source`s
struct pamh_getenv_status pamh_get_complete_env(pam_handle_t* NNULLABLE handle,
                                                struct passwd* NNULLABLE pw,
                                                enum SessionType session_typ);

void free_envlist(char* NULLABLE* NNULLABLE envlist);
pam_handle_t* NULLABLE get_pamh(char* NNULLABLE user, char* NNULLABLE passwd);

#endif /* PAM_H */
