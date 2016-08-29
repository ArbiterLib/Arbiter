#ifndef ARBITER_TYPES_H
#define ARBITER_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/**
 * Frees an Arbiter object.
 *
 * This is valid to call with a NULL pointer.
 */
void ArbiterFree (void *object);

/**
 * Creates a copy of an Arbiter object.
 *
 * The returned pointer must be freed with ArbiterFree().
 */
void *ArbiterCreateCopy (const void *object);

/**
 * Returns whether two Arbiter objects are equal.
 */
bool ArbiterEqual (const void *left, const void *right);

/**
 * Creates a NUL-terminated string description of an Arbiter object.
 *
 * The returned C string must be freed with free().
 */
char *ArbiterCreateDescription (const void *object);

#ifdef __cplusplus
}
#endif

#endif
