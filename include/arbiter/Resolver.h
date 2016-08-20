#ifndef ARBITER_RESOLVER_H
#define ARBITER_RESOLVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <arbiter/Dependency.h>
#include <arbiter/Value.h>
#include <arbiter/Version.h>

#include <stdbool.h>

/**
 * A dependency resolver which contains context about how to evaluate the
 * dependency graph.
 */
typedef struct ArbiterResolver ArbiterResolver;

/**
 * Represents a request for the list of dependencies needed by a specific
 * version of a project.
 */
typedef struct
{
  /**
   * The dependency resolver which is asking for this information.
   */
  const ArbiterResolver *resolver;

  /**
   * The project whose dependencies need to be fetched.
   */
  const ArbiterProjectIdentifier *project;

  /**
   * The specific version of the project for which the dependency list should be
   * fetched.
   */
  const ArbiterSelectedVersion *selectedVersion;
} ArbiterDependencyListFetch;

/**
 * Represents a request for all of the available versions of a project.
 */
typedef struct
{
  /**
   * The dependency resolver which is asking for this information.
   */
  const ArbiterResolver *resolver;

  /**
   * The project whose list of available versions should be fetched.
   */
  const ArbiterProjectIdentifier *project;
} ArbiterAvailableVersionsFetch;

/**
 * User-provided behaviors for how dependency resolution should work.
 */
typedef struct
{
  /**
   * A thread-safe, re-entrant function which fetches a project version's
   * dependencies according to the given request, then invokes either
   * `onSuccess` or `onError`.
   *
   * Failure to invoke one of the completion callbacks will result in a memory
   * leak.
   */
  void (*fetchDependencyList)(
    ArbiterDependencyListFetch fetch,
    void (*onSuccess)(ArbiterDependencyListFetch fetch, const ArbiterDependencyList *fetchedList),
    void (*onError)(ArbiterDependencyListFetch fetch)
  );

  /**
   * A thread-safe, re-entrant function which fetches a project's available
   * versions according to the given request.
   *
   * The implementor should invoke `onNext` as each version is found, and then
   * `onCompleted` after all versions have been enumerated, or else `onError` if
   * an error occurs before enumeration can finish.
   *
   * Failure to invoke one of the completion callbacks will result in a memory
   * leak.
   */
  void (*fetchAvailableVersions)(
    ArbiterAvailableVersionsFetch fetch,
    void (*onNext)(ArbiterAvailableVersionsFetch fetch, const ArbiterSelectedVersion *nextVersion),
    void (*onCompleted)(ArbiterAvailableVersionsFetch fetch),
    void (*onError)(ArbiterAvailableVersionsFetch fetch)
  );
} ArbiterResolverBehaviors;

/**
 * User-provided callbacks to invoke when dependency resolution has progressed.
 */
typedef struct
{
  /**
   * A thread-safe function which will be invoked when the specified version of
   * `project` has been picked as a fully-resolved dependency.
   */
  void (*onSuccess)(
    const ArbiterResolver *resolver,
    const ArbiterProjectIdentifier *project,
    const ArbiterSelectedVersion *selectedVersion
  );

  /**
   * A thread-safe function which will be invoked when an error has occurred
   * trying to resolve the next dependency.
   */
  void (*onError)(
    const ArbiterResolver *resolver,
    const char *message
  );
} ArbiterResolverCallbacks;

/**
 * Creates a dependency resolver, implemented using the given behaviors, which
 * will attempt to pick compatible versions of all dependencies in
 * `dependencyList` and transitive dependencies thereof.
 *
 * The returned dependency resolver must be freed with ArbiterFreeResolver().
 */
ArbiterResolver *ArbiterCreateResolver (ArbiterResolverBehaviors behaviors, const ArbiterDependencyList *dependencyList, ArbiterUserValue context);

/**
 * Returns any context data which was provided to ArbiterCreateResolver().
 *
 * The returned pointer is only guaranteed to remain valid for the current
 * scope.
 */
const void *ArbiterResolverContext (const ArbiterResolver *resolver);

/**
 * Returns whether all dependencies have finished being resolved to specific
 * versions.
 */
bool ArbiterResolvedAllDependencies (const ArbiterResolver *resolver);

/**
 * Begins resolving the next unresolved dependency. Once resolution has
 * completed or an error has occurred, one of the given callbacks will be
 * invoked (possibly on another thread).
 *
 * After starting to resolve a dependency, the dependency resolver object must
 * not be freed with ArbiterFreeResolver() until one of the callbacks has been
 * invoked.
 *
 * It is invalid to invoke this function if ArbiterResolvedAllDependencies()
 * returns true for the same dependency resolver.
 */
void ArbiterStartResolvingNextDependency (ArbiterResolver *resolver, ArbiterResolverCallbacks callbacks);

/**
 * Releases the memory associated with a dependency resolver.
 */
void ArbiterFreeResolver (ArbiterResolver *resolver);

#ifdef __cplusplus
}
#endif

#endif
