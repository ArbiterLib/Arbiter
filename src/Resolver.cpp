#include "Resolver-inl.h"

using namespace Arbiter;

ArbiterResolver *ArbiterCreateResolver (ArbiterResolverBehaviors behaviors, const ArbiterDependencyList *dependencyList, ArbiterUserValue context)
{
  return new ArbiterResolver(behaviors, *dependencyList, SharedUserValue(context));
}

const void *ArbiterResolverContext (const ArbiterResolver *resolver)
{
  return resolver->_context.data();
}

bool ArbiterResolvedAllDependencies (const ArbiterResolver *resolver)
{
  assert(false);
  return false;
}

void ArbiterStartResolvingNextDependency (ArbiterResolver *resolver, ArbiterResolverCallbacks callbacks)
{
  assert(false);
}

void ArbiterFreeResolver (ArbiterResolver *resolver)
{
  delete resolver;
}
