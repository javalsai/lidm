#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "chvt.h"

static char* vterms[] = {"/dev/tty", "/dev/tty0", "/dev/vc/0", "/dev/systty",
                         "/dev/console"};

int chvt_str(char* str) {
  char* err;
  errno = 0;
  long i = strtol(str, &err, 10);
  if (errno) {
    perror("strol");
    return -1;
  }
  // I'm not gonna elaborate on this....
  if (i > INT_MAX || i < INT_MIN || *err) return -1;

  return chvt((int)i);
}

int chvt(int n) {
  fprintf(stderr, "activating vt %d\n", n);
  char c = 0;
  for (size_t i = 0; i < sizeof(vterms) / sizeof(vterms[0]); i++) {
    int fd = open(vterms[i], O_RDWR);
    if (fd >= 0 && isatty(fd) && ioctl(fd, KDGKBTYPE, &c) == 0 && c < 3) {
      if (ioctl(fd, VT_ACTIVATE, n) < 0 || ioctl(fd, VT_WAITACTIVE, n) < 0) {
        fprintf(stderr, "Couldn't activate vt %d\n", n);
        return -1;
      }
      return 0;
    }
    close(fd);
  }

  fprintf(stderr, "Couldn't get a file descriptor referring to the console.\n");
  return -1;
}
