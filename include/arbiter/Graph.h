#ifndef ARBITER_GRAPH_H
#define ARBITER_GRAPH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

// forward declarations
struct ArbiterProjectIdentifier;
struct ArbiterRequirement;
struct ArbiterResolvedDependency;
struct ArbiterSelectedVersion;

/**
 * Represents a fully consistent, resolved dependency graph, preserving
 * relationships between dependencies.
 */
typedef struct ArbiterResolvedDependencyGraph ArbiterResolvedDependencyGraph;

/**
 * Creates an empty resolved dependency graph.
 *
 * The returned graph must be freed with ArbiterFree().
 */
ArbiterResolvedDependencyGraph *ArbiterResolvedDependencyGraphCreate (void);

/**
 * Creates a resolved dependency graph based upon `baseGraph`, but excluding any
 * nodes and edges which are not reachable from `roots`.
 *
 * In other words, this creates a new sub-graph which is rooted at `roots`. The
 * new roots do not necessarily have to be siblings in `baseGraph`.
 *
 * This operation is mostly useful to create a filtered graph which can then be
 * passed as the `initialGraph` to ArbiterCreateResolver(). For example, if an
 * end user only wants to upgrade a specific set of projects, this function
 * could be used to filter out those projects from an existing graph, and
 * then resolve only those specific projects from scratch.
 *
 * The returned graph must be freed with ArbiterFree().
 */
ArbiterResolvedDependencyGraph *ArbiterResolvedDependencyGraphCopyWithNewRoots (const ArbiterResolvedDependencyGraph *baseGraph, const struct ArbiterProjectIdentifier * const *roots, size_t rootCount);

/**
 * Attempts to add a root node into the dependency graph, without making it
 * inconsistent.
 *
 * If the given dependency refers to a project which already exists in the
 * graph, this will attempt to intersect the version requirements of both.
 *
 * Returns whether the addition succeeded. If `false` is returned and `error` is
 * not NULL, it may be set to a string describing the error, which the caller is
 * responsible for freeing.
 */
bool ArbiterResolvedDependencyGraphAddRoot (ArbiterResolvedDependencyGraph *graph, const struct ArbiterResolvedDependency *node, const struct ArbiterRequirement *requirement, char **error);

/**
 * Attempts to add an edge (dependency relationship) into the dependency graph,
 * from `dependent` to `dependency`, without making it inconsistent.
 *
 * If `dependency` refers to a project which already exists in the graph, this
 * will attempt to intersect the version requirements of both.
 *
 * Returns whether the addition succeeded. If `false` is returned and `error` is
 * not NULL, it may be set to a string describing the error, which the caller is
 * responsible for freeing.
 */
bool ArbiterResolvedDependencyGraphAddEdge (ArbiterResolvedDependencyGraph *graph, const struct ArbiterProjectIdentifier *dependent, const struct ArbiterResolvedDependency *dependency, const struct ArbiterRequirement *requirement, char **error);

/**
 * Returns the number of unique nodes in the given graph, for use with
 * ArbiterResolvedDependencyGraphGetAll().
 *
 * The returned count may be invalidated if the graph is modified.
 */
size_t ArbiterResolvedDependencyGraphCount (const ArbiterResolvedDependencyGraph *graph);

/**
 * Copies all of the resolved dependencies in the given graph into the C array
 * `buffer`, which must have enough space to contain
 * ArbiterResolvedDependencyGraphCount() elements.
 *
 * This operation does not guarantee a specific ordering to the copied items.
 *
 * The copied objects must be individually freed with ArbiterFree().
 */
void ArbiterResolvedDependencyGraphCopyAll (const ArbiterResolvedDependencyGraph *graph, struct ArbiterResolvedDependency **buffer);

/**
 * Returns the version which was selected for the given project in the
 * dependency graph, or NULL if the project is not part of the graph.
 *
 * The returned pointer is guaranteed to remain valid until the
 * ArbiterResolvedDependencyGraph it was obtained from is modified or freed.
 */
const struct ArbiterSelectedVersion *ArbiterResolvedDependencyGraphProjectVersion (const ArbiterResolvedDependencyGraph *graph, const struct ArbiterProjectIdentifier *project);

/**
 * Returns the requirement which is attached to the given project in the
 * dependency graph, or NULL if the project is not part of the graph.
 *
 * The returned pointer is guaranteed to remain valid until the
 * ArbiterResolvedDependencyGraph it was obtained from is modified or freed.
 */
const struct ArbiterRequirement *ArbiterResolvedDependencyGraphProjectRequirement (const ArbiterResolvedDependencyGraph *graph, const struct ArbiterProjectIdentifier *project);

/**
 * Returns the number of dependencies that the given project has in the graph,
 * or 0 if the project does not exist in the graph.
 *
 * The returned count may be invalidated if the graph is modified.
 */
size_t ArbiterResolvedDependencyGraphCountDependencies (const ArbiterResolvedDependencyGraph *graph, const struct ArbiterProjectIdentifier *project);

/**
 * Copies pointers to the projects representing the given project's dependencies
 * into the C array `buffer`, which must have enough space to contain
 * ArbiterResolvedDependencyGraphCountDependencies() elements.
 *
 * This operation guarantees that project identifiers will appear in the buffer
 * in ascending order.
 *
 * The copied pointers are guaranteed to remain valid until the
 * ArbiterResolvedDependencyGraph they were obtained from is modified or freed.
 */
void ArbiterResolvedDependencyGraphGetAllDependencies (const ArbiterResolvedDependencyGraph *graph, const struct ArbiterProjectIdentifier *project, const struct ArbiterProjectIdentifier **buffer);

/**
 * Enumerates a resolved dependency graph in "install order," where all projects
 * listed within one phase may be safely installed in parallel with respect to
 * each another, and the projects within _each successive phase_ must be
 * installed only after the projects in _all previous phases_ have been
 * completely installed.
 */
typedef struct ArbiterResolvedDependencyInstaller ArbiterResolvedDependencyInstaller;

/**
 * Creates an installer for the given resolved dependency graph.
 *
 * The dependency graph can be safely freed after calling this function.
 *
 * The returned installer must be freed with ArbiterFree().
 */
ArbiterResolvedDependencyInstaller *ArbiterResolvedDependencyInstallerCreate (const ArbiterResolvedDependencyGraph *graph);

/**
 * Returns the number of phases that the installer has, for use with
 * ArbiterResolvedDependencyInstallerCountInPhase() and
 * ArbiterResolvedDependencyInstallerGetAllInPhase().
 */
size_t ArbiterResolvedDependencyInstallerPhaseCount (const ArbiterResolvedDependencyInstaller *installer);

/**
 * Returns the number of resolved dependencies that exist within the given
 * zero-based installer phase, for use with
 * ArbiterResolvedDependencyInstallerGetAllInPhase().
 *
 * This represents a set of projects which can safely be built or installed in
 * parallel with one another.
 */
size_t ArbiterResolvedDependencyInstallerCountInPhase (const ArbiterResolvedDependencyInstaller *installer, size_t phaseIndex);

/**
 * Copies pointers to the resolved dependencies which exist at the given
 * zero-based installer phase into the C array `buffer`, which must have enough
 * space to contain ArbiterResolvedDependencyInstallerCountInPhase() elements.
 *
 * This operation guarantees that resolved dependencies will appear in the
 * buffer in ascending order of their project identifiers.
 *
 * The copied pointers are guaranteed to remain valid until the
 * ArbiterResolvedDependencyInstaller they were obtained from is freed.
 */
void ArbiterResolvedDependencyInstallerGetAllInPhase (const ArbiterResolvedDependencyInstaller *installer, size_t phaseIndex, const struct ArbiterResolvedDependency **buffer);

#ifdef __cplusplus
}
#endif

#endif
