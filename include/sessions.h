#ifndef SESSIONSH_
#define SESSIONSH_

#include <sys/types.h>
#include <unistd.h>

#include "macros.h"
#include "util.h"

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
  char** args;
  int arg_count;
};

struct session_exec {
  enum ExecType typ;
  union {
    char* shell;
    struct desktop_exec desktop;
  };
};

static inline struct session_exec session_exec_shell(char* shell) {
  return (struct session_exec){
      .typ = EXEC_SHELL,
      .shell = shell,
  };
}

static inline struct session_exec session_exec_desktop(int arg_count,
                                                       char** args) {
  return (struct session_exec){
      .typ = EXEC_DESKTOP,
      .desktop =
          {
              .args = args,
              .arg_count = arg_count,
          },
  };
}

static inline int session_exec_exec(struct session_exec* exec,
                                    char** NNULLABLE envlist) {
  switch (exec->typ) {
    case EXEC_SHELL:
      return execle(exec->shell, exec->shell, NULL, envlist);
    case EXEC_DESKTOP:
      return execve(exec->desktop.args[0], exec->desktop.args, envlist);
    default:
      __builtin_unreachable();
  }
}

struct session {
  char* NNULLABLE name;
  struct session_exec exec;
  enum SessionType type;
};

struct Vector get_avaliable_sessions();

#endif
