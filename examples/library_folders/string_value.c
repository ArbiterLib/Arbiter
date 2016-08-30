#include "string_value.h"

#include <stdio.h>
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
  if (!copy) {
    perror("Could not allocate space for string copy");
    abort();
  }

  strncpy(copy, str, length);

  copy[length] = '\0';
  return copy;
}

static size_t hash (const void *ptr)
{
  size_t value = 0;

  const char *str = ptr;

  // how not to write a hash function
  while (*str != '\0') {
    value += *(str++);
  }

  return value;
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
    .hash = &hash,
    .createDescription = &createDescription,
    .destructor = &free,
  };

  return value;
}
