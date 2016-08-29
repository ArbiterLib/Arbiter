#include "asprintf.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

int custom_asprintf (char **ret, const char *format, ...)
{
  assert(ret);
  assert(format);

  va_list sizeArgs;
  va_start(sizeArgs, format);

  va_list printArgs;
  va_copy(printArgs, sizeArgs);

  int result;

  int size = vsnprintf(NULL, 0, format, sizeArgs);
  va_end(sizeArgs);

  if (size < 0) {
    result = size;
    goto cleanup;
  }

  char *buffer = malloc(size + 1);
  if (!buffer) {
    result = -1;
    goto cleanup;
  }

  result = vsnprintf(buffer, size + 1, format, printArgs);
  if (result >= 0) {
    *ret = buffer;
  } else {
    free(buffer);
  }

cleanup:
  va_end(printArgs);

  if (result < 0) {
    *ret = NULL;
  }

  return result;
}
