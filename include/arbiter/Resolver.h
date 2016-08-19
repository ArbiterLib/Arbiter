#ifndef ARBITER_RESOLVER_H
#define ARBITER_RESOLVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <arbiter/Dependency.h>
#include <arbiter/Value.h>
#include <arbiter/Version.h>

#include <stdbool.h>

typedef struct ArbiterResolver ArbiterResolver;

typedef struct
{
  const ArbiterResolver *resolver;
  const ArbiterProjectIdentifier *project;
  const ArbiterSelectedVersion *selectedVersion;
} ArbiterDependencyListFetch;

typedef struct
{
  const ArbiterResolver *resolver;
  const ArbiterProjectIdentifier *project;
} ArbiterAvailableVersionsFetch;

typedef struct
{
  void (*fetchDependencyList)(
    ArbiterDependencyListFetch fetch,
    void (*onSuccess)(ArbiterDependencyListFetch fetch, const ArbiterDependencyList *fetchedList),
    void (*onError)(ArbiterDependencyListFetch fetch)
  );

  void (*fetchAvailableVersions)(
    ArbiterAvailableVersionsFetch fetch,
    void (*onNext)(ArbiterAvailableVersionsFetch fetch, const ArbiterSelectedVersion *nextVersion),
    void (*onCompleted)(ArbiterAvailableVersionsFetch fetch),
    void (*onError)(ArbiterAvailableVersionsFetch fetch)
  );
} ArbiterResolverBehaviors;

typedef struct
{
  void (*onSuccess)(
    const ArbiterResolver *resolver,
    const ArbiterProjectIdentifier *project,
    const ArbiterSelectedVersion *selectedVersion
  );

  void (*onError)(
    const ArbiterResolver *resolver,
    const char *message
  );
} ArbiterResolverCallbacks;

ArbiterResolver *ArbiterCreateResolver (ArbiterResolverBehaviors behaviors, const ArbiterDependencyList *dependencyList, ArbiterUserValue context);
const void *ArbiterResolverContext (const ArbiterResolver *resolver);
bool ArbiterResolvedAllDependencies (const ArbiterResolver *resolver);
void ArbiterStartResolvingNextDependency (ArbiterResolver *resolver, ArbiterResolverCallbacks callbacks);
void ArbiterFreeResolver (ArbiterResolver *resolver);

#ifdef __cplusplus
}
#endif

#endif
