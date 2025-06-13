#ifndef UIH_
#define UIH_

#include "config.h"
#include "util.h"

void setup(struct config* config);
int load(struct Vector* users, struct Vector* sessions);
void print_err(const char*);
void print_errno(const char*);

#endif
