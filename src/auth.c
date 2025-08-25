#include <grp.h>
#include <pwd.h>
#include <security/pam_misc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>

#include "auth.h"
#include "config.h"
#include "sessions.h"
#include "ui.h"
#include "unistd.h"
#include "util.h"

// constants for exec string parsing
// ARG_LENGTH is the initial length of a parsed argument
#define MAX_ARGS 100
#define ARG_LENGTH 64

// parse Exec=/bin/prog arg1 arg2\ with\ spaces
void free_parsed_args(char** args, int arg_count) {
  if (!args) return;
  for (int i = 0; i < arg_count; i++) {
    free(args[i]);
  }
  free((void*)args);
}

/* small closure-like struct to pass state to helper functions */
struct ctx {
  char** pcur;
  size_t* pcur_len;
  size_t* pcur_cap;
  char*** pargv;
  int* pargc;
};
/* append_char(state, ch) -> 0 on error, 1 on success */
int append_char(struct ctx* st, char ch) {
  char** pcur = st->pcur;
  size_t* plen = st->pcur_len;
  size_t* pcap = st->pcur_cap;
  if (*plen + 1 >= *pcap) {
    size_t newcap = *pcap ? (*pcap) * 2 : ARG_LENGTH;
    char* cur = (char*)realloc(*pcur, newcap);
    if (!cur) return 0;
    *pcur = cur;
    *pcap = newcap;
  }
  (*pcur)[(*plen)++] = ch;
  return 1;
}

/* push_arg(state) -> 0 on error, 1 on success */
int push_arg(struct ctx* st) {
  char** pcur = st->pcur;
  size_t* plen = st->pcur_len;
  size_t* pcap = st->pcur_cap;
  char*** pargv = st->pargv;
  int* pargc = st->pargc;

  if (*pargc > MAX_ARGS) {
    return 1;
  }
  if (!*pcur) {
    char* empty = strdup("");
    if (!empty) return 0;
    char** na = (char**)realloc((void*)*pargv, sizeof(char*) * ((*pargc) + 1));
    if (!na) {
      free(empty);
      return 0;
    }
    *pargv = na;
    (*pargv)[(*pargc)++] = empty;
    return 1;
  }
  if (!append_char(st, '\0')) return 0;
  char* final = (char*)realloc(*pcur, *plen);
  if (!final) final = *pcur;
  *pcur = NULL;
  *plen = 0;
  *pcap = 0;
  char** na = (char**)realloc((void*)*pargv, sizeof(char*) * ((*pargc) + 1));
  if (!na) {
    free(final);
    return 0;
  }
  *pargv = na;
  (*pargv)[(*pargc)++] = final;
  return 1;
}

/* Return codes:
   0 = success
   1 = bad args
   2 = memory
   3 = syntax

  Important: call free_parsed_args afterwards to free the passed ***args
*/
// NOLINTBEGIN(readability-function-cognitive-complexity)
int parse_exec_string(const char* exec_s, char*** args, int* arg_count) {
  if (!exec_s || !args || !arg_count) return 1;
  *args = NULL;
  *arg_count = 0;

  size_t len = strlen(exec_s);
  size_t idx = 0;
  char* cur = NULL;
  size_t cur_len = 0;
  size_t cur_cap = 0;
  char** argv = NULL;
  int argc = 0;
  int in_quote = 0;

  struct ctx ctx;
  ctx.pcur = &cur;
  ctx.pcur_len = &cur_len;
  ctx.pcur_cap = &cur_cap;
  ctx.pargv = &argv;
  ctx.pargc = &argc;

  while (idx < len) {
    char cur_c = exec_s[idx];
    if (!in_quote && (cur_c == ' ' || cur_c == '\t' || cur_c == '\n')) {
      if (cur_cap) {
        if (!push_arg(&ctx)) goto nomem;
      }
      idx++;
      continue;
    }
    if (!in_quote && cur_c == '"') {
      in_quote = 1;
      idx++;
      continue;
    }
    if (in_quote && cur_c == '"') {
      in_quote = 0;
      idx++;
      continue;
    }

    if (cur_c == '\\') {
      if (idx + 1 >= len) goto syntax_err;
      if (!append_char(&ctx, exec_s[idx + 1])) goto nomem;
      idx += 2;
      continue;
    }

    if (cur_c == '%') {
      if (idx + 1 >= len) goto syntax_err;
      if (exec_s[idx + 1] == '%') {
        if (!append_char(&ctx, '%')) goto nomem;
        idx += 2;
        continue;
      }
      /* drop any %X */
      idx += 2;
      continue;
    }

    if (!append_char(&ctx, cur_c)) goto nomem;
    idx++;
  }

  if (in_quote) goto syntax_err;
  if (cur_cap) {
    if (!push_arg(&ctx)) goto nomem;
  }
  char** na = (char**)realloc((void*)argv, sizeof(char*) * (argc + 1));
  if (!na) goto nomem;
  argv = na;
  argv[argc] = NULL;

  *args = argv;
  *arg_count = argc;
  return 0;

nomem:
  if (cur) free(cur);
  free_parsed_args(argv, argc);
  *args = NULL;
  *arg_count = 0;
  return 2;

syntax_err:
  if (cur) free(cur);
  free_parsed_args(argv, argc);
  *args = NULL;
  *arg_count = 0;
  return 3;
}
// NOLINTEND(readability-function-cognitive-complexity)

int pam_conversation(int num_msg, const struct pam_message** msg,
                     struct pam_response** resp, void* appdata_ptr) {
  struct pam_response* reply =
      (struct pam_response*)malloc(sizeof(struct pam_response) * num_msg);
  if (!reply) {
    return PAM_BUF_ERR;
  }
  for (size_t i = 0; i < num_msg; i++) {
    reply[i].resp = NULL;
    reply[i].resp_retcode = 0;
    if (msg[i]->msg_style == PAM_PROMPT_ECHO_OFF ||
        msg[i]->msg_style == PAM_PROMPT_ECHO_ON) {
      char* input = (char*)appdata_ptr;
      reply[i].resp = strdup(input);
    }
  }
  *resp = reply;
  return PAM_SUCCESS;
}

#ifndef PAM_SERVICE_FALLBACK
  #define PAM_SERVICE_FALLBACK "login"
#endif

#define CHECK_PAM_RET(call) \
  ret = (call);             \
  if (ret != PAM_SUCCESS) { \
    pam_end(pamh, ret);     \
    return NULL;            \
  }

void clear_screen() {
  printf("\x1b[H\x1b[J");
}

pam_handle_t* get_pamh(char* user, char* passwd) {
  pam_handle_t* pamh = NULL;
  struct pam_conv pamc = {pam_conversation, (void*)passwd};
  int ret;

  char* pam_service_override = getenv("LIDM_PAM_SERVICE");
  char* pam_service_name =
      pam_service_override ? pam_service_override : PAM_SERVICE_FALLBACK;

  CHECK_PAM_RET(pam_start(pam_service_name, user, &pamc, &pamh))
  CHECK_PAM_RET(pam_authenticate(pamh, 0))
  CHECK_PAM_RET(pam_acct_mgmt(pamh, 0))
  CHECK_PAM_RET(pam_setcred(pamh, PAM_ESTABLISH_CRED))
  CHECK_PAM_RET(pam_open_session(pamh, 0))
  CHECK_PAM_RET(pam_setcred(pamh, PAM_REINITIALIZE_CRED))

  return pamh;
}
#undef CHECK_PAM_RET

void* shmalloc(size_t size) {
  return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
              -1, 0);
}

void sourceFileTry(char* file) {
  FILE* file2source = fopen(file, "r");
  if (file2source == NULL) return;

  char* line = NULL;
  size_t len = 0;
  ssize_t read;

  while ((read = getline(&line, &len, file2source)) != -1) {
    if (read == 0 || (read > 0 && *line == '#')) continue;
    if (line[read - 1] == '\n') line[read - 1] = '\0';

    /* printf("Retrieved line of length %zu:\n", read); */
    /* printf("%s\n", line); */
    for (size_t i = 1; i < read; i++) {
      if (line[i] == '=') {
        /* printf("FOUND '='!\n"); */
        line[i] = '\0';
        setenv(line, &line[i + 1], 1);
        break;
      }
    }
  }

  if (line) free(line);
  (void)fclose(file2source);
}

void moarEnv(char* user, struct session session, struct passwd* pw,
             struct config* config) {
  if (chdir(pw->pw_dir) == -1) print_errno("can't chdir to user home");

  setenv("HOME", pw->pw_dir, true);
  setenv("USER", pw->pw_name, true);
  setenv("SHELL", pw->pw_shell, true);
  // TERM
  setenv("LOGNAME", pw->pw_name, true);
  // MAIL?

  // PATH?

  char* xdg_session_type = "unknown";
  if (session.type == SHELL) xdg_session_type = "tty";
  if (session.type == XORG) xdg_session_type = "x11";
  if (session.type == WAYLAND) xdg_session_type = "wayland";
  setenv("XDG_SESSION_TYPE", xdg_session_type, true);

  printf("\n\n\n\n\x1b[1m");
  for (size_t i = 0; i < config->behavior.source.length; i++) {
    /* printf("DEBUG(source)!!!! %d %s\n", i, (char*)vec_get(&behavior->source,
     * i)); */
    sourceFileTry((char*)vec_get(&config->behavior.source, i));
  }
  /* printf("\n"); */

  if (pw->pw_dir) {
    const char* home = pw->pw_dir;
    size_t home_len = strlen(home);

    for (size_t i = 0; i < config->behavior.user_source.length; i++) {
      const char* filename = (char*)vec_get(&config->behavior.user_source, i);
      size_t filename_len = strlen(filename);

      size_t path_len = home_len + 1 + filename_len + 1; // nullbyte and slash
      char* path = malloc(path_len);
      if (!path) continue; // can't bother

      memcpy(path, home, home_len);
      path[home_len] = '/'; // assume pw_dir doesn't start with '/' :P
      memcpy(&path[home_len + 1], filename, filename_len);
      path[path_len - 1] = '\0';

      sourceFileTry(path);
      free(path);
    }
  }

  /*char *buf;*/
  /*size_t bsize = snprintf(NULL, 0, "/run/user/%d", pw->pw_uid) + 1;*/
  /*buf = malloc(bsize);*/
  /*snprintf(buf, bsize, "/run/user/%d", pw->pw_uid);*/
  /*setenv("XDG_RUNTIME_DIR", buf, true);*/
  /*setenv("XDG_SESSION_CLASS", "user", true);*/
  /*setenv("XDG_SESSION_ID", "1", true);*/
  /*setenv("XDG_SESSION_DESKTOP", , true);*/
  /*setenv("XDG_SEAT", "seat0", true);*/
}

// NOLINTBEGIN(readability-function-cognitive-complexity)
bool launch(char* user, char* passwd, struct session session, void (*cb)(void),
            struct config* config) {
  struct passwd* pw = getpwnam(user);
  if (pw == NULL) {
    print_err("could not get user info");
    return false;
  }

  pam_handle_t* pamh = get_pamh(user, passwd);
  if (pamh == NULL) {
    print_err("error on pam authentication");
    return false;
  }

  bool* reach_session = shmalloc(sizeof(bool));
  if (reach_session == NULL) {
    perror("error allocating shared memory");
    return false;
  }
  *reach_session = false;

  uint pid = fork();
  if (pid == 0) { // child
    char* term = NULL;
    char* getterm = getenv("TERM");
    // TODO: handle malloc error
    if (getterm != NULL) term = strdup(getterm);
    if (clearenv() != 0) {
      print_errno("clearenv");
      _exit(EXIT_FAILURE);
    }

    char** envlist = pam_getenvlist(pamh);
    if (envlist == NULL) {
      print_errno("pam_getenvlist");
      _exit(EXIT_FAILURE);
    }
    for (size_t i = 0; envlist[i] != NULL; i++) {
      putenv(envlist[i]);
    }
    // FIXME: path hotfix
    putenv("PATH=/bin:/usr/bin");
    if (term != NULL) {
      setenv("TERM", term, true);
      free(term);
    }

    free((void*)envlist);
    moarEnv(user, session, pw, config);

    // TODO: chown stdin to user
    // does it inherit stdin from parent and
    // does parent need to reclaim it after
    // this dies?

    if (setgid(pw->pw_gid) == -1) {
      print_errno("setgid");
      _exit(EXIT_FAILURE);
    }
    if (initgroups(user, pw->pw_gid) == -1) {
      print_errno("initgroups");
      _exit(EXIT_FAILURE);
    }

    if (setuid(pw->pw_uid) == -1) {
      perror("setuid");
      _exit(EXIT_FAILURE);
    }

    if (cb != NULL) cb();

    *reach_session = true;

    // TODO: test existence of executable with TryExec
    char** args = NULL;
    int arg_count = 0;
    int parse_status = parse_exec_string(session.exec, &args, &arg_count);
    if (parse_status != 0 || arg_count == 0 || !args[0]) {
      (void)fprintf(stderr, "failure parsing exec string '%s': %d\n",
                    session.exec ? session.exec : "NULL", parse_status);
      free_parsed_args(args, arg_count);
      return false;
    }

    printf("\x1b[0m");
    // NOLINTNEXTLINE(bugprone-branch-clone)
    if (session.type == SHELL) {
      clear_screen();
      (void)fflush(stdout);
      execvp(args[0], args);
    } else if (session.type == XORG || session.type == WAYLAND) {
      clear_screen();
      (void)fflush(stdout);
      execvp(args[0], args);
    }
    free_parsed_args(args, arg_count);
    perror("execl error");
    (void)fputs("failure calling session\n", stderr);
  } else {
    pid_t child_pid = (pid_t)pid;
    waitpid(child_pid, NULL, 0);

    pam_setcred(pamh, PAM_DELETE_CRED);
    pam_close_session(pamh, 0);
    pam_end(pamh, PAM_SUCCESS);

    if (*reach_session == false) return false;
    exit(0);
  }

  return true;
}
// NOLINTEND(readability-function-cognitive-complexity)
