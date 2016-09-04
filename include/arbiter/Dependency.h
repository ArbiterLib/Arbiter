#ifndef ARBITER_DEPENDENCY_H
#define ARBITER_DEPENDENCY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <arbiter/Value.h>

#include <stddef.h>

// forward declarations
struct ArbiterRequirement;
struct ArbiterSelectedVersion;

/**
 * An opaque value which identifies a project participating in dependency
 * resolution.
 */
typedef struct ArbiterProjectIdentifier ArbiterProjectIdentifier;

/**
 * Creates a project identifier from the given opaque data.
 *
 * The returned identifier must be freed with ArbiterFree().
 */
ArbiterProjectIdentifier *ArbiterCreateProjectIdentifier (ArbiterUserValue value);

/**
 * Returns the opaque data which was provided to ArbiterCreateProjectIdentifier().
 *
 * The returned pointer is only guaranteed to remain valid for the current
 * scope.
 */
const void *ArbiterProjectIdentifierValue (const ArbiterProjectIdentifier *projectIdentifier);

/**
 * Represents a dependency specification, which consists of a project identifier
 * and a version requirement.
 */
typedef struct ArbiterDependency ArbiterDependency;

/**
 * Creates a dependency which specifies a version requirement of the given
 * project.
 *
 * The returned dependency must be freed with ArbiterFree().
 */
ArbiterDependency *ArbiterCreateDependency (const ArbiterProjectIdentifier *projectIdentifier, const struct ArbiterRequirement *requirement);

/**
 * Returns the project identified by this dependency. 
 *
 * The returned pointer is only guaranteed to remain valid for the current
 * scope.
 */
const ArbiterProjectIdentifier *ArbiterDependencyProject (const ArbiterDependency *dependency);

/**
 * Returns the version requirement of this dependency. 
 *
 * The returned pointer is only guaranteed to remain valid for the current
 * scope.
 */
const struct ArbiterRequirement *ArbiterDependencyRequirement (const ArbiterDependency *dependency);

/**
 * Represents a list of dependencies.
 */
typedef struct ArbiterDependencyList ArbiterDependencyList;

/**
 * Creates a dependency list which wraps a C array of ArbiterDependency objects.
 *
 * The objects in the C array can be safely freed after calling this function.
 *
 * The returned list must be freed with ArbiterFree().
 */
ArbiterDependencyList *ArbiterCreateDependencyList (const ArbiterDependency * const *dependencies, size_t count);

/**
 * Represents a dependency which has been resolved to a specific version.
 */
typedef struct ArbiterResolvedDependency ArbiterResolvedDependency;

/**
 * Creates a fully resolved dependency referring to the specified version of the
 * given project.
 *
 * The returned dependency must be freed with ArbiterFree().
 */
ArbiterResolvedDependency *ArbiterCreateResolvedDependency (const ArbiterProjectIdentifier *project, const struct ArbiterSelectedVersion *version);

/**
 * Returns the project this resolved dependency refers to.
 *
 * The returned pointer is only guaranteed to remain valid for the current
 * scope.
 */
const ArbiterProjectIdentifier *ArbiterResolvedDependencyProject (const ArbiterResolvedDependency *dependency);

/**
 * Returns the version which was selected for this resolved dependency.
 *
 * The returned pointer is only guaranteed to remain valid for the current
 * scope.
 */
const struct ArbiterSelectedVersion *ArbiterResolvedDependencyVersion (const ArbiterResolvedDependency *dependency);

/**
 * Represents a resolved dependency graph, preserving relationships between
 * dependencies.
 *
 * Resolved graphs are ordered in such a way that projects without any
 * dependencies all exist at depth 0, projects which depend only upon those
 * exist at depth 1, projects which depend upon depth 0 or 1 exist at depth 2,
 * etc.
 *
 * In other words, the graph can be thought of as being "install-ordered," where
 * the projects listed within one depth index may be safely installed in
 * parallel with respect to each another, and the projects within _each
 * successive depth_ must be installed only after the projects in _all previous
 * depths_ have been completely installed.
 */
typedef struct ArbiterResolvedDependencyGraph ArbiterResolvedDependencyGraph;

/**
 * Returns the number of unique nodes in the given graph, for use with
 * ArbiterResolvedDependencyGraphGetAll().
 */
size_t ArbiterResolvedDependencyGraphCount (const ArbiterResolvedDependencyGraph *graph);

/**
 * Copies pointers to all of the resolved dependencies in the given graph, into
 * the C array `buffer`, which must have enough space to contain
 * ArbiterResolvedDependencyGraphCount() elements.
 *
 * This operation does not guarantee a specific ordering to the copied items.
 *
 * The copied pointers are guaranteed to remain valid until the
 * ArbiterResolvedDependencyGraph they were obtained from is freed.
 */
void ArbiterResolvedDependencyGraphGetAll (const ArbiterResolvedDependencyGraph *graph, const ArbiterResolvedDependency **buffer);

/**
 * Returns the depth of the graph, for use with
 * ArbiterResolvedDependencyGraphCountAtDepth() and
 * ArbiterResolvedDependencyGraphGetAllAtDepth().
 */
size_t ArbiterResolvedDependencyGraphDepth (const ArbiterResolvedDependencyGraph *graph);

/**
 * Returns the number of projects which exist at the given zero-based "depth
 * index."
 *
 * This represents a set of projects which safely be built or installed in
 * parallel with one another.
 */
size_t ArbiterResolvedDependencyGraphCountAtDepth (const ArbiterResolvedDependencyGraph *graph, size_t depthIndex);

/**
 * Copies pointers to the resolved dependencies which exist at the given
 * zero-based "depth index," into the C array `buffer`, which must have enough
 * space to contain ArbiterResolvedDependencyGraphCountAtDepth() elements.
 *
 * This operation does not guarantee a specific ordering to the copied items.
 *
 * The copied pointers are guaranteed to remain valid until the
 * ArbiterResolvedDependencyGraph they were obtained from is freed.
 */
void ArbiterResolvedDependencyGraphGetAllAtDepth (const ArbiterResolvedDependencyGraph *graph, size_t depthIndex, const ArbiterResolvedDependency **buffer);

#ifdef __cplusplus
}
#endif

#endif
