#ifndef CHVTH_
#define CHVTH_

#include <fcntl.h>
#ifdef __FreeBSD__
#include <sys/kbio.h>
#include <sys/consio.h>
#else
#include <linux/kd.h>
#include <linux/vt.h>
#endif
#include <sys/ioctl.h>
#include <unistd.h>

/**
 * @brief change foreground virtual terminal to `n`
 *
 * @param n virtual terminal number
 * @return int non-negative value on success
 */
int chvt(int n);
/**
 * @brief change foreground virtual terminal to `str`
 *
 * @param str virtual terminal number (string)
 * @return int non-negative value on success
 */
int chvt_str(char* str);

#endif
