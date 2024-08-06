#ifndef _CHVTH_
#define _CHVTH_

#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define KDGKBTYPE 0x4b33
#define VT_ACTIVATE 0x5606
#define VT_WAITACTIVE 0x5607

/**
 * @brief change foreground virtual terminal to `n`
 *
 * @param n virtual terminal number
 * @return int non-negative value on success
 */
int chvt(int n);

#endif
