#include <stdlib.h>
#include <string.h>

#include <util.h>

void strcln(char **dest, const char *source) {
  *dest = malloc(strlen(source) + sizeof(char));
  strcpy(*dest, source);
}
