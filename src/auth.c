#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <sys/wait.h>

#include <auth.h>
#include <stdlib.h>

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

bool check_passwd(char *user, char *passwd) {
  pam_handle_t *pamh = NULL;
  struct pam_conv pamc = {pam_conversation, (void *)passwd};
  int retval;

  retval = pam_start("login", user, &pamc, &pamh);
  if (retval != PAM_SUCCESS) {
    return false;
  }

  retval = pam_authenticate(pamh, 0);
  if (retval != PAM_SUCCESS) {
    pam_end(pamh, retval);
    return false;
  }

  retval = pam_acct_mgmt(pamh, 0);
  if (retval != PAM_SUCCESS) {
    pam_end(pamh, retval);
    return false;
  }

  pam_end(pamh, PAM_SUCCESS);
  return true;
}

/*void run(char *user, char *passwd, char *type, char *command) {*/
/*  int pipefd[2];*/
/*  pid_t pid;*/
/**/
/*  if (pipe(pipefd) == -1) {*/
/*    perror("pipe");*/
/*    exit(EXIT_FAILURE);*/
/*  }*/
/**/
/*  if ((pid = fork()) == -1) {*/
/*    perror("fork");*/
/*    exit(EXIT_FAILURE);*/
/*  }*/
/**/
/*  if (pid == 0) {*/
/*    close(pipefd[0]);*/
/*    write(pipefd[1], passwd, strlen(passwd));*/
/*    close(pipefd[1]);*/
/*    exit(EXIT_SUCCESS);*/
/*  } else {*/
/*    close(pipefd[1]);*/
/**/
/*    if (dup2(pipefd[0], STDIN_FILENO) == -1) {*/
/*      perror("dup2");*/
/*      exit(EXIT_FAILURE);*/
/*    }*/
/*    close(pipefd[0]);*/
/**/
/*    execlp("su", "-", user, type, command, (char *)NULL);*/
/*    perror("execlp");*/
/*    exit(EXIT_FAILURE);*/
/*  }*/
/*}*/

void run(char *user, char *passwd, char *type, char *command) {
  int pipefd[2];
  pipe(pipefd);
  close(STDIN_FILENO);
  dup2(pipefd[0], STDIN_FILENO);
  write(pipefd[1], passwd, strlen(passwd));
  write(pipefd[1], "\n", 1);
  char *const args[] = { "-", user, type, command, NULL };
  execvp("su", args);
  exit(1);
}

bool launch(char *user, char *passwd, struct session session, void (*cb)(void)) {
  if (!check_passwd(user, passwd))
    return false;

  if (cb != NULL)
    cb();

  if (session.type == SHELL) {
    system("clear");
    run(user, passwd, "-s", session.exec);
  } else if (session.type == XORG || session.type == WAYLAND) {
    system("clear");
    run(user, passwd, "-c", session.exec);
  }

  return true;
}
