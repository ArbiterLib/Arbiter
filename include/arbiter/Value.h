#ifndef ARBITER_VALUE_H
#define ARBITER_VALUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/**
 * Represents an arbitrary value that can be associated with Arbiter data types
 * and functionality.
 *
 * For example, ArbiterProjectIdentifiers are defined by providing an opaque
 * user value.
 */
typedef struct
{
  /**
   * The underlying data object.
   *
   * This object should be considered to be owned by Arbiter as soon as the
   * ArbiterUserValue is passed into any API. It will eventually be cleaned up
   * by the library through invocation of the provided `destructor`.
   */
  void *data;

  /**
   * An equality operation over two data objects.
   *
   * This must not be NULL.
   */
  bool (*equalTo)(const void *first, const void *second);

  /**
   * Returns whether `first` is less than (should be ordered before) `second`.
   *
   * This must not be NULL.
   */
  bool (*lessThan)(const void *first, const void *second);

  /**
   * A cleanup function to call when the ArbiterUserValue is done being used.
   *
   * This may be NULL.
   */
  void (*destructor)(void *data);

  /**
   * An operation to convert this data object to a string. The returned value
   * must be dynamically allocated and support being destroyed with free().
   *
   * This may be NULL.
   */
  char *(*createDescription)(const void *data);
} ArbiterUserValue;

#ifdef __cplusplus
}
#endif

#endif
