#ifndef ARBITER_DEPENDENCY_H
#define ARBITER_DEPENDENCY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <arbiter/Requirement.h>

/**
 * An opaque value which identifies a project participating in dependency
 * resolution.
 */
typedef struct ArbiterProjectIdentifier ArbiterProjectIdentifier;

/**
 * Creates a project identifier from the given opaque data.
 *
 * The returned identifier must be freed with ArbiterFreeProjectIdentifier().
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
 * Releases the memory associated with a project identifier.
 */
void ArbiterFreeProjectIdentifier (ArbiterProjectIdentifier *projectIdentifier);

/**
 * Represents a dependency specification, which consists of a project identifier
 * and a version requirement.
 */
typedef struct ArbiterDependency ArbiterDependency;

/**
 * Creates a dependency which specifies a version requirement of the given
 * project.
 *
 * The returned dependency must be freed with ArbiterFreeDependency().
 */
ArbiterDependency *ArbiterCreateDependency (const ArbiterProjectIdentifier *projectIdentifier, const ArbiterRequirement *requirement);

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
const ArbiterRequirement *ArbiterDependencyRequirement (const ArbiterDependency *dependency);

/**
 * Releases the memory associated with a dependency object.
 */
void ArbiterFreeDependency (ArbiterDependency *dependency);

/**
 * Represents a list of dependencies.
 */
typedef struct ArbiterDependencyList ArbiterDependencyList;

/**
 * Creates a dependency list which wraps a C array of ArbiterDependency objects.
 */
ArbiterDependencyList *ArbiterCreateDependencyList (const ArbiterDependency *dependencies, size_t count);

/**
 * Releases the memory associated with a dependency list.
 */
void ArbiterFreeDependencyList (ArbiterDependencyList *dependencyList);

#ifdef __cplusplus
}
#endif

#endif
