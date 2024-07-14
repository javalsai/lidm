#include <grp.h>
#include <pwd.h>
#include <security/_pam_types.h>
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <sys/wait.h>

#include <auth.h>
#include <stdlib.h>
#include <ui.h>

int pam_conversation(int num_msg, const struct pam_message **msg,
                     struct pam_response **resp, void *appdata_ptr) {
  struct pam_response *reply =
      (struct pam_response *)malloc(sizeof(struct pam_response) * num_msg);
  for (int i = 0; i < num_msg; i++) {
    reply[i].resp = NULL;
    reply[i].resp_retcode = 0;
    if (msg[i]->msg_style == PAM_PROMPT_ECHO_OFF ||
        msg[i]->msg_style == PAM_PROMPT_ECHO_ON) {
      char *input = (char *)appdata_ptr;
      reply[i].resp = strdup(input);
    }
  }
  *resp = reply;
  return PAM_SUCCESS;
}

pam_handle_t *get_pamh(char *user, char *passwd) {
  pam_handle_t *pamh = NULL;
  struct pam_conv pamc = {pam_conversation, (void *)passwd};
  int retval;

  retval = pam_start("login", user, &pamc, &pamh);
  if (retval != PAM_SUCCESS) {
    return NULL;
  }

  retval = pam_authenticate(pamh, 0);
  if (retval != PAM_SUCCESS) {
    pam_end(pamh, retval);
    return NULL;
  }

  retval = pam_acct_mgmt(pamh, 0);
  if (retval != PAM_SUCCESS) {
    pam_end(pamh, retval);
    return NULL;
  }

  return pamh;
}

bool launch(char *user, char *passwd, struct session session,
            void (*cb)(void)) {
  struct passwd *pw = getpwnam(user);
  if (pw == NULL) {
    print_err("could not get user info");
    return false;
  }

  gid_t *groups;
  int ngroups = 0;
  getgrouplist(user, pw->pw_gid, NULL, &ngroups);
  groups = malloc(ngroups * sizeof(gid_t));
  if (groups == NULL) {
    print_err("malloc error");
    return false;
  }
  if (getgrouplist(user, pw->pw_gid, groups, &ngroups) == -1) {
    free(groups);
    print_err("error fetching groups");
    return false;
  }

  pam_handle_t *pamh = get_pamh(user, passwd);
  if (pamh == NULL) {
    print_err("error on pam authentication");
    return false;
  }

  char **envlist = pam_getenvlist(pamh);
  if (envlist == NULL) {
    print_err("error getting pam env");
    return false;
  }

  // point of no return
  int setgrps_ret = setgroups(ngroups, groups);
  free(groups);
  if (setgrps_ret == -1) {
    perror("setgroups");
    exit(EXIT_FAILURE);
  }

  if (setgid(pw->pw_gid) == -1) {
    perror("setgid");
    exit(EXIT_FAILURE);
  }

  if (setuid(pw->pw_uid) == -1) {
    perror("setuid");
    exit(EXIT_FAILURE);
  }

  for (char **env = envlist; *env != NULL; env++) {
    putenv(*env);
  }

  if (cb != NULL)
    cb();

  pam_end(pamh, PAM_SUCCESS);
  if (session.type == SHELL) {
    system("clear");
    execl(session.exec, session.exec, NULL);
  } else if (session.type == XORG || session.type == WAYLAND) {
    system("clear");
    execl(session.exec, session.exec, NULL);
  }

  return true;
}
