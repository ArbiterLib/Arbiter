#include "strerror.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *strerror_copy (const char *prefix, int errnum)
{
  if (!prefix) {
    prefix = "";
  }

  size_t prefixLen = strlen(prefix);

  const char *errString = strerror(errnum);
  if (!errString) {
    errString = "(unknown)";
  }

  size_t errLen = strlen(errString);

  char *buffer = malloc(prefixLen + errLen + 1);
  if (!buffer) {
    perror("Could not allocate space for error string");
    abort();
  }

  strncpy(buffer, prefix, prefixLen);
  strncpy(buffer + prefixLen, errString, errLen);
  buffer[prefixLen + errLen] = '\0';

  return buffer;
}
