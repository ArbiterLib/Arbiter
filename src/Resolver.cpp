#include "Resolver-inl.h"

using namespace Arbiter;

ArbiterResolver *ArbiterCreateResolver (ArbiterResolverBehaviors behaviors, const ArbiterDependencyList *dependencyList, ArbiterUserValue context)
{
  return new ArbiterResolver(std::move(behaviors), *dependencyList, SharedUserValue(std::move(context)));
}

const void *ArbiterResolverContext (const ArbiterResolver *resolver)
{
  return resolver->_context.data();
}

bool ArbiterResolvedAllDependencies (const ArbiterResolver *resolver)
{
  return resolver->_remainingDependencies._dependencies.empty();
}

void ArbiterStartResolvingNextDependency (ArbiterResolver *resolver, ArbiterResolverCallbacks callbacks)
{
  resolver->resolveNext(std::move(callbacks));
}

void ArbiterFreeResolver (ArbiterResolver *resolver)
{
  delete resolver;
}

void ArbiterResolver::resolveNext (ArbiterResolverCallbacks callbacks)
{
  assert(false);
}
