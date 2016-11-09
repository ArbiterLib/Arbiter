#ifndef ARBITER_RESOLVER_H
#define ARBITER_RESOLVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <arbiter/Value.h>

#include <stdbool.h>

// forward declarations
struct ArbiterDependencyList;
struct ArbiterProjectIdentifier;
struct ArbiterResolvedDependencyGraph;
struct ArbiterSelectedVersion;
struct ArbiterSelectedVersionList;

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
  struct ArbiterDependencyList *(*createDependencyList)(const ArbiterResolver *resolver, const struct ArbiterProjectIdentifier *project, const struct ArbiterSelectedVersion *selectedVersion, char **error);

  /**
   * Requests the list of versions available for a given project.
   *
   * Returns a version list, or NULL if an error occurs. Arbiter will be
   * responsible for freeing the returned version list object. If returning
   * NULL, `error` may be set to a string describing the error which occurred,
   * in which case Arbiter will be responsible for freeing the string.
   */
  struct ArbiterSelectedVersionList *(*createAvailableVersionsList)(const ArbiterResolver *resolver, const struct ArbiterProjectIdentifier *project, char **error);

  /**
   * Requests the selected version which corresponds to the given metadata, in
   * the context of the given project.
   *
   * This behavior can be used to implement lookup of versions which are not
   * known in advance (i.e., those which would not appear in the result of
   * `createAvailableVersionsList`). For example, it is impractical to list all
   * commit hashes from a version control system, but they could be looked up by
   * hash here.
   *
   * This behavior is optional, and may be set to NULL if unsupported or
   * unnecessary.
   *
   * Returns a selected version, or NULL if one corresponding to the metadata
   * could not be found.
   */
  struct ArbiterSelectedVersion *(*createSelectedVersionForMetadata)(const ArbiterResolver *resolver, const struct ArbiterProjectIdentifier *project, const void *metadata);
} ArbiterResolverBehaviors;

/**
 * Creates a dependency resolver, implemented using the given behaviors, which
 * will attempt to add compatible versions of all dependencies in
 * `dependenciesToResolve` into the `initialGraph`.
 *
 * If `initialGraph` is NULL or empty, this is like creating a new graph which
 * is populated with everything in `dependenciesToResolve` and all transitive
 * dependencies thereof.
 *
 * Otherwise, the listed dependencies are _unified_ with whatever is already in
 * the graph. Projects in and transitive dependencies of `dependenciesToResolve`
 * which are not already in the graph will be added. For any dependency which
 * _is_ already in the graph, the version from the graph must be satisfied by
 * the updated dependency's requirement, or else resolution will fail.
 *
 * The returned dependency resolver must be freed with ArbiterFree().
 *
 * To understand how to use the parameters to this function, let's look at a few
 * use cases:
 *
 *  1. **Resolving dependencies for the first time.** In this circumstance,
 *     simply provide an `initialGraph` of `NULL`, and list all dependencies in
 *     `dependenciesToResolve`. The result will be a fully-resolved graph for
 *     all of those dependencies.
 *  2. **Adding a new project to an already-resolved graph.** Simply pass the
 *     existing graph in as the `initialGraph`, then list _only_ the project(s)
 *     to be added in `dependenciesToResolve`. When resolution has completed,
 *     the versions of existing projects in the graph will be unchanged, but the
 *     new project and its new transitive dependencies will have been added.
 *  3. **Upgrading a project in an already-resolved graph.** First, use
 *     ArbiterResolvedDependencyGraphCopyWithNewRoots() to copy all root
 *     projects that should remain _the same_ (i.e., those which should not be
 *     upgraded). Then, provide the projects which _should_ be updated as
 *     `dependenciesToResolve`. When resolution has completed, the
 *     `dependenciesToResolve` will be in the graph at the highest possible
 *     version, and the versions of the other, "pinned" projects will remain the
 *     same.
 */
ArbiterResolver *ArbiterCreateResolver (ArbiterResolverBehaviors behaviors, const struct ArbiterResolvedDependencyGraph *initialGraph, const struct ArbiterDependencyList *dependenciesToResolve, ArbiterUserContext context);

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
 * Returns the graph of resolved dependencies, or NULL if an error occurred. The
 * caller is responsible for freeing the returned graph. If NULL is returned and
 * `error` is not NULL, it may be set to a string describing the error, which
 * the caller is responsible for freeing.
 */
struct ArbiterResolvedDependencyGraph *ArbiterResolverCreateResolvedDependencyGraph (ArbiterResolver *resolver, char **error);

#ifdef __cplusplus
}
#endif

#endif
