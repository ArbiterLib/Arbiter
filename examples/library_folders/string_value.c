#include "string_value.h"

#include <stdlib.h>
#include <string.h>

static bool equalTo (const void *first, const void *second)
{
  return strcmp(first, second) == 0;
}

static bool lessThan (const void *first, const void *second)
{
  return strcmp(first, second) < 0;
}

static char *copyString (const char *str, size_t length)
{
  char *copy = malloc(length + 1);
  strncpy(copy, str, length);

  copy[length] = '\0';
  return copy;
}

static char *createDescription (const void *str)
{
  return copyString(str, strlen(str));
}

ArbiterUserValue string_value_from_string (const char *str, size_t length)
{
  ArbiterUserValue value = {
    .data = copyString(str, length),
    .equalTo = &equalTo,
    .lessThan = &lessThan,
    .destructor = &free,
    .createDescription = &createDescription,
  };

  return value;
}
