// TODO: rewrite properly
// NOLINTBEGIN(clang-diagnostic-nullability-completeness)
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "desktop_exec.h"
#include "macros.h"

// constants for exec string parsing
#define MAX_ARGS 100
// ARG_LENGTH is the initial length of a parsed argument
#define ARG_LENGTH 64

// returns NULL on any error
// otherwise it returns the absolute path of the program that MUST BE FREED
char* NULLABLE search_path(const char* NNULLABLE for_binary) {
  if (strchr(for_binary, '/') != NULL) {
    // skip absolute paths
    return strdup(for_binary);
  }
  char* path_env = getenv("PATH");
  if (!path_env) return NULL;
  char* path = strdup(path_env);
  if (!path) return NULL;

  char* tok = strtok(path, ":");
  while (tok) {
    char* bin_path;
    asprintf(&bin_path, "%s/%s", tok, for_binary);
    if (!bin_path) {
      free(path);
      return NULL;
    }

    struct stat stat_buf;
    if (stat(bin_path, &stat_buf) == 0) {
      // TODO: check exec bit ig
      // if(stat_buf.) {}
      free(path);
      return bin_path;
    }

    free(bin_path);
    tok = strtok(NULL, ":");
  }

  free(path);
  return NULL;
}

// returns -1 on exec failure and -2 on search failure
int execvpe_desktop(char** args, char* NNULLABLE* NNULLABLE envlist) {
  char* new_arg = search_path(args[0]);
  if (!new_arg) return -2;

  free(args[0]);
  args[0] = new_arg;

  int status = execve(args[0], args, envlist);
  free(new_arg);

  return status;
}

// parse Exec=/bin/prog arg1 arg2\ with\ spaces
void free_parsed_args(int arg_count, char** args) {
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
int parse_exec_string(const char* exec_s, int* arg_count, char*** args) {
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
  free_parsed_args(argc, argv);
  *args = NULL;
  *arg_count = 0;
  return 2;

syntax_err:
  if (cur) free(cur);
  free_parsed_args(argc, argv);
  *args = NULL;
  *arg_count = 0;
  return 3;
}
// NOLINTEND(readability-function-cognitive-complexity)
// NOLINTEND(clang-diagnostic-nullability-completeness)
