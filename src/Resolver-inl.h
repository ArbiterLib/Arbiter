#ifndef ARBITER_RESOLVER_INL_H
#define ARBITER_RESOLVER_INL_H

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include "Dependency-inl.h"
#include "Resolver.h"
#include "Value-inl.h"

struct ArbiterResolver
{
  public:
    ArbiterResolverBehaviors _behaviors;
    ArbiterDependencyList _remainingDependencies;
    Arbiter::SharedUserValue _context;

    ArbiterResolver (ArbiterResolverBehaviors behaviors, ArbiterDependencyList dependencyList, Arbiter::SharedUserValue context)
      : _behaviors(std::move(behaviors))
      , _remainingDependencies(std::move(dependencyList))
      , _context(std::move(context))
    {}

    void resolveNext (ArbiterResolverCallbacks callbacks);
};

#endif
