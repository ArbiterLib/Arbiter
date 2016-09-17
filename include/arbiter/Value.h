#ifndef ARBITER_VALUE_H
#define ARBITER_VALUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

/**
 * Represents an arbitrary value type that can be associated with Arbiter data
 * types and functionality.
 *
 * For example, ArbiterProjectIdentifiers are defined by providing a user value
 * type.
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
   * Generates a hash of the data object. The hash does not need to be
   * cryptographically secure.
   *
   * This must not be NULL.
   */
  size_t (*hash)(const void *first);

  /**
   * An operation to convert this data object to a string. The returned value
   * must be dynamically allocated and support being destroyed with free().
   *
   * This may be NULL.
   */
  char *(*createDescription)(const void *data);

  /**
   * A cleanup function to call when the ArbiterUserValue is done being used.
   *
   * This may be NULL.
   */
  void (*destructor)(void *data);
} ArbiterUserValue;

/**
 * Represents opaque data that can be passed to Arbiter data types and
 * functionality, then later retrieved.
 *
 * This type is used instead of raw pointers to make memory management safer.
 */
typedef struct
{
  /**
   * The underlying data pointer.
   *
   * This pointer should be considered to be owned by Arbiter as soon as the
   * ArbiterUserContext is passed into any API. It will eventually be cleaned up
   * by the library through invocation of the provided `destructor`.
   */
  void *data;

  /**
   * A cleanup function to call when the ArbiterUserContext is done being used.
   *
   * This may be NULL.
   */
  void (*destructor)(void *data);
} ArbiterUserContext;

#ifdef __cplusplus
}
#endif

#endif
