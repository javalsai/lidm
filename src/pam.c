#include <pwd.h>
#include <security/_pam_types.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "macros.h"
#include "pam.h"
#include "sessions.h"
#include "ui.h"

struct envpair {
  const char* NNULLABLE name;
  char* NULLABLE value;
};

char* NULLABLE make_env_kv(const char* NNULLABLE key, char* NNULLABLE value) {
  char* buf = NULL;
  asprintf(&buf, "%s=%s", key, value);
  return buf;
}

void free_envlist(char** NNULLABLE envlist) {
  for (char** ptr = envlist; *ptr; ptr++)
    free(*ptr);
  free((void*)envlist);
}

// NULL when allocation failure
// in any case, envlist would be freed after this function
char** NULLABLE merge_envlist(char** NNULLABLE envlist, struct envpair extra[],
                              size_t extra_len) {
  size_t envlist_len = 0;
  while (envlist[envlist_len])
    envlist_len++;

  size_t nonnullelems = 0;
  for (size_t i = 0; i < extra_len; i++) {
    if (extra[i].value) nonnullelems++;
  }

  size_t new_envlist_len = envlist_len + nonnullelems + 1;
  char** new_envlist =
      (char**)realloc((void*)envlist, sizeof(char*) * new_envlist_len);
  if (!new_envlist) {
    free_envlist(envlist);
    return NULL;
  }

  // NOLINTNEXTLINE(readability-identifier-length)
  size_t k = 0;
  for (size_t i = 0; i < extra_len; i++) {
    if (!extra[i].value) continue;
    char* env_kv = make_env_kv(extra[i].name, extra[i].value);
    if (!env_kv) goto free_new_envlist_extra;
    new_envlist[envlist_len + k++] = env_kv;
  }

  new_envlist[envlist_len + nonnullelems] = NULL;
  return new_envlist;

free_new_envlist_extra:
  for (size_t j = 0; j < envlist_len + k; j++) {
    free(new_envlist[envlist_len + j]);
  }
  free((void*)new_envlist);
  return NULL;
}

char* NULLABLE xdg_ssession_type_str(enum SessionType typ) {
  char* xdg_session_type = NULL;
  if (typ == SHELL) xdg_session_type = "tty";
  if (typ == XORG) xdg_session_type = "x11";
  if (typ == WAYLAND) xdg_session_type = "wayland";
  return xdg_session_type;
}

#define FAIL_ALLOC()                                                  \
  {                                                                   \
    return (struct pamh_getenv_status){.error_flag = PAMH_ERR_ALLOC}; \
  }
#define FAIL(ERR, ERRFN)                                                       \
  {                                                                            \
    return (struct pamh_getenv_status){.error_flag = (ERR), .errfn = (ERRFN)}; \
  }

struct pamh_getenv_status pamh_get_complete_env(pam_handle_t* handle,
                                                struct passwd* NNULLABLE pw,
                                                enum SessionType session_typ) {
  char** raw_envlist = pam_getenvlist(handle);
  if (!raw_envlist) FAIL(PAMH_ERR_ERRNO, "pam_getenvlist");

  struct envpair extra_env[] = {
      {"TERM", getenv("TERM")},
      {"PATH", getenv("PATH")},
      {"HOME", pw->pw_dir},
      {"USER", pw->pw_name},
      {"SHELL", pw->pw_shell},
      {"LOGNAME", pw->pw_name},
      {"XDG_SESSION_TYPE", xdg_ssession_type_str(session_typ)}};

  char** envlist = merge_envlist(raw_envlist, extra_env, LEN(extra_env));
  if (!envlist) FAIL_ALLOC();
  return (struct pamh_getenv_status){
      .error_flag = PAMH_ERR_NOERR,
      .envlist = envlist,
  };
}

#undef FAIL
#undef FAIL_ALLOC

///////////////

struct pam_conv_data {
  char* password;
  void (*display_pam_msg)(const char* msg, int msg_style);
};

int pam_conversation(int num_msg, const struct pam_message** msg,
                     struct pam_response** resp, void* appdata_ptr) {
  struct pam_response* reply = malloc(sizeof(struct pam_response) * num_msg);
  if (!reply) {
    return PAM_BUF_ERR;
  }

  struct pam_conv_data* conv_data = (struct pam_conv_data*)appdata_ptr;

  for (int i = 0; i < num_msg; i++) {
    reply[i].resp = NULL;
    reply[i].resp_retcode = 0;

    switch (msg[i]->msg_style) {
      case PAM_PROMPT_ECHO_OFF:
      case PAM_PROMPT_ECHO_ON:
        reply[i].resp = strdup(conv_data->password);
        if (!reply[i].resp) {
          for (int j = 0; j < i; j++)
            free(reply[j].resp);
          free(reply);
          return PAM_BUF_ERR;
        }
        break;

      case PAM_TEXT_INFO:
      case PAM_ERROR_MSG:
        if (conv_data->display_pam_msg && msg[i]->msg) {
          conv_data->display_pam_msg(msg[i]->msg, msg[i]->msg_style);
        }
        break;

      default:
        break;
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
pam_handle_t* get_pamh(char* user, char* passwd) {
  pam_handle_t* pamh = NULL;
  struct pam_conv_data conv_data = {.password = passwd,
                                    .display_pam_msg = print_pam_msg};
  struct pam_conv pamc = {pam_conversation, (void*)&conv_data};
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
