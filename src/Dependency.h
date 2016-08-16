#ifndef ARBITER_DEPENDENCY_H
#define ARBITER_DEPENDENCY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Requirement.h"

typedef struct ArbiterProjectIdentifier ArbiterProjectIdentifier;

ArbiterProjectIdentifier *ArbiterCreateProjectIdentifier (ArbiterUserValue value);
const void *ArbiterProjectIdentifierValue (const ArbiterProjectIdentifier *projectIdentifier);
void ArbiterFreeProjectIdentifier (ArbiterProjectIdentifier *projectIdentifier);

typedef struct ArbiterDependency ArbiterDependency;

ArbiterDependency *ArbiterCreateDependency (const ArbiterProjectIdentifier *projectIdentifier, const ArbiterRequirement *requirement);
const ArbiterProjectIdentifier *ArbiterDependencyProject (const ArbiterDependency *dependency);
const ArbiterRequirement *ArbiterDependencyRequirement (const ArbiterDependency *dependency);
void ArbiterFreeDependency (ArbiterDependency *dependency);

typedef struct ArbiterDependencyList ArbiterDependencyList;

ArbiterDependencyList *ArbiterCreateDependencyList (const ArbiterDependency *dependencies, size_t count);
void ArbiterFreeDependencyList (ArbiterDependencyList *dependencyList);

#ifdef __cplusplus
}
#endif

#endif
