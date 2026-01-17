#include <errno.h>
#include <stddef.h>
#include <sys/wait.h>
#include <unistd.h>

static void handle_sigterm(int) {
  signal(SIGTERM, SIG_IGN);

  kill(-getpgrp(), SIGTERM);

  int status;
  while (waitpid(-1, &status, 0) > 0 || errno == EINTR) {
  }

  _exit(0);
}

void setup_sigterm() {
  setpgid(0, 0);

  struct sigaction sa;
  sa.sa_handler = handle_sigterm;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGTERM, &sa, NULL);
}
