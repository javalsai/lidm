#include "chvt.h"

static char *vterms[] = {"/dev/tty", "/dev/tty0", "/dev/vc/0", "/dev/systty",
                         "/dev/console"};

int chvt(int n) {
  char c = 0;
  for (int i = 0; i < sizeof(vterms) / sizeof(vterms[0]); i++) {
    int fd = open(vterms[i], O_RDWR);
    if (fd >= 0 && isatty(fd) && ioctl(fd, KDGKBTYPE, &c) == 0 && c < 3) {
      if (ioctl(fd, VT_ACTIVATE, n) < 0 || ioctl(fd, VT_WAITACTIVE, n) < 0) {
        printf("Couldn't active vt %d\n", n);
        return -1;
      }
      return 0;
    }
    close(fd);
  }

  printf("Couldn't get a file descriptor referring to the console.\n");
  return -1;
}
