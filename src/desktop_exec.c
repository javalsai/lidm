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
#include "util/path.h"

// constants for exec string parsing
#define MAX_ARGS 100
// ARG_LENGTH is the initial length of a parsed argument
#define ARG_LENGTH 64

char* NULLABLE desktop_as_cmdline(char** args) {
  if (args[0] == NULL) return NULL;
  size_t fmtd_len = 0;

  char** ptr = args;
  while (*ptr) {
    fmtd_len += strlen(*ptr) + 1;
    ptr++;
  }
  fmtd_len -= 1;

  char* fmt_cmdline = malloc(fmtd_len + 1);
  if (!fmt_cmdline) return NULL;

  size_t fmting_len = 0;
  ptr = args;
  while (*ptr) {
    char* nbyte = stpcpy(&fmt_cmdline[fmting_len], *ptr);
    *nbyte = ' ';

    fmting_len += nbyte - &fmt_cmdline[fmting_len] + 1;
    ptr++;
  }
  fmt_cmdline[fmting_len - 1] = '\0';

  return fmt_cmdline;
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
