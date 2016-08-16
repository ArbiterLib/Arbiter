#ifndef ARBITER_VALUE_H
#define ARBITER_VALUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef struct
{
  void *data;
  bool (*equals)(const void *first, const void *second);
  void (*destructor)(void *data);

  // optional
  char *(*createDescription)(const void *data);
} ArbiterUserValue;

#ifdef __cplusplus
}
#endif

#endif
