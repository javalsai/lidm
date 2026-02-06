#ifndef SESSIONSH_
#define SESSIONSH_

#include <sys/types.h>
#include <unistd.h>

#include "macros.h"
#include "util/vec.h"

enum SessionType {
  XORG,
  WAYLAND,
  SHELL,
};

enum ExecType {
  EXEC_SHELL,
  EXEC_DESKTOP,
};

struct desktop_exec {
  char* NULLABLE* NNULLABLE args;
  int arg_count;
};

struct session_exec {
  enum ExecType typ;
  union {
    char* NNULLABLE shell;
    struct desktop_exec desktop;
  };
};

static inline struct session_exec session_exec_shell(char* NNULLABLE shell) {
  return (struct session_exec){
      .typ = EXEC_SHELL,
      .shell = shell,
  };
}

static inline struct session_exec session_exec_desktop(
    int arg_count, char* NULLABLE* NNULLABLE args) {
  return (struct session_exec){
      .typ = EXEC_DESKTOP,
      .desktop =
          {
              .args = args,
              .arg_count = arg_count,
          },
  };
}

struct session {
  char* NNULLABLE name;
  struct session_exec exec;
  enum SessionType type;
};

struct Vector get_avaliable_sessions();
int session_exec_exec(struct session_exec* NNULLABLE exec,
                                    char* NULLABLE* NNULLABLE envp);
int session_exec_login_through_shell(struct session_exec* NNULLABLE exec,
                                     char* NULLABLE* NNULLABLE envp);

#endif
