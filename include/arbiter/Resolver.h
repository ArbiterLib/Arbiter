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
 * User-provided behaviors for how dependency resolution should work.
 */
typedef struct
{
  /**
   * Requests the list of dependencies needed by a specific version of
   * a project.
   *
   * Returns a dependency list, or NULL if an error occurs. Arbiter will be
   * responsible for freeing the returned dependency list object. If returning
   * NULL, `error` may be set to a string describing the error which occurred,
   * in which case Arbiter will be responsible for freeing the string.
   */
  ArbiterDependencyList *(*createDependencyList)(const ArbiterResolver *resolver, const ArbiterProjectIdentifier *project, const ArbiterSelectedVersion *selectedVersion, char **error);

  /**
   * Requests the list of versions available for a given project.
   *
   * Returns a version list, or NULL if an error occurs. Arbiter will be
   * responsible for freeing the returned version list object. If returning
   * NULL, `error` may be set to a string describing the error which occurred,
   * in which case Arbiter will be responsible for freeing the string.
   */
  ArbiterSelectedVersionList *(*createAvailableVersionsList)(const ArbiterResolver *resolver, const ArbiterProjectIdentifier *project, char **error);
} ArbiterResolverBehaviors;

/**
 * Creates a dependency resolver, implemented using the given behaviors, which
 * will attempt to pick compatible versions of all dependencies in
 * `dependencyList` and transitive dependencies thereof.
 *
 * The returned dependency resolver must be freed with ArbiterFreeResolver().
 */
ArbiterResolver *ArbiterCreateResolver (ArbiterResolverBehaviors behaviors, const ArbiterDependencyList *dependencyList, const void *context);

/**
 * Returns any context data which was provided to ArbiterCreateResolver().
 *
 * The returned pointer is only guaranteed to remain valid for the current
 * scope.
 */
const void *ArbiterResolverContext (const ArbiterResolver *resolver);

/**
 * Attempts to resolve all dependencies.
 *
 * Returns the list of resolved dependencies, or NULL if an error occurred. The
 * caller is responsible for freeing the returned list. If NULL is returned and
 * `error` is not NULL, it may be set to a string describing the error, which
 * the caller is responsible for freeing.
 */
ArbiterResolvedDependencyList *ArbiterResolverCreateResolvedDependencyList (ArbiterResolver *resolver, char **error);

/**
 * Releases the memory associated with a dependency resolver.
 */
void ArbiterFreeResolver (ArbiterResolver *resolver);

#ifdef __cplusplus
}
#endif

#endif
