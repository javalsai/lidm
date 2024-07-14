#ifndef _UIH_
#define _UIH_

#include <config.h>

void setup(struct config);
int load(struct users_list*, struct sessions_list*);
void print_err(const char*);
void print_errno(const char*);

#endif
