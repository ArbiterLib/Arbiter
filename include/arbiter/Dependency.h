#ifndef ARBITER_DEPENDENCY_H
#define ARBITER_DEPENDENCY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

// forward declarations
struct ArbiterProjectIdentifier;
struct ArbiterRequirement;
struct ArbiterSelectedVersion;

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
ArbiterDependency *ArbiterCreateDependency (const struct ArbiterProjectIdentifier *projectIdentifier, const struct ArbiterRequirement *requirement);

/**
 * Returns the project identified by this dependency. 
 *
 * The returned pointer is only guaranteed to remain valid for the current
 * scope.
 */
const struct ArbiterProjectIdentifier *ArbiterDependencyProject (const ArbiterDependency *dependency);

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
ArbiterResolvedDependency *ArbiterCreateResolvedDependency (const struct ArbiterProjectIdentifier *project, const struct ArbiterSelectedVersion *version);

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

#ifdef __cplusplus
}
#endif

#endif
