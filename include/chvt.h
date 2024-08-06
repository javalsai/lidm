#ifndef _CHVTH_
#define _CHVTH_

#include <fcntl.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <sys/ioctl.h>
#include <unistd.h>

/**
 * @brief change foreground virtual terminal to `n`
 *
 * @param n virtual terminal number
 * @return int non-negative value on success
 */
int chvt(int n);

#endif
