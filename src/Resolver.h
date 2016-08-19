#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include "Dependency.h"
#include "Future.h"
#include "Hash.h"
#include "Value.h"
#include "Version.h"

#include <arbiter/Resolver.h>

#include <mutex>
#include <unordered_map>

namespace Arbiter {
namespace Resolver {

struct ResolvedDependency
{
  ArbiterProjectIdentifier projectIdentifier;
  ArbiterSelectedVersion selectedVersion;

  static ResolvedDependency takeOwnership (ArbiterDependencyListFetch fetch) noexcept;

  bool operator== (const ResolvedDependency &other) const noexcept;
};

}
}

namespace std {

template<>
struct hash<Arbiter::Resolver::ResolvedDependency>
{
  public:
    size_t operator() (const Arbiter::Resolver::ResolvedDependency &fetch) const
    {
      return Arbiter::hashOf(fetch.projectIdentifier)
        ^ Arbiter::hashOf(fetch.selectedVersion);
    }
};

}

struct ArbiterResolver
{
  public:
    Arbiter::SharedUserValue _context;

    ArbiterResolver (ArbiterResolverBehaviors behaviors, ArbiterDependencyList dependencyList, Arbiter::SharedUserValue context)
      : _context(std::move(context))
      , _behaviors(std::move(behaviors))
      , _remainingDependencies(std::move(dependencyList))
    {}

    Arbiter::Future<ArbiterDependencyList> fetchDependencyList (Arbiter::Resolver::ResolvedDependency fetch);

    bool resolvedAll () const noexcept;
    Arbiter::Future<Arbiter::Resolver::ResolvedDependency> resolveNext ();

  private:
    const ArbiterResolverBehaviors _behaviors;
    ArbiterDependencyList _remainingDependencies;

    std::mutex _fetchesMutex;
    std::unordered_map<Arbiter::Resolver::ResolvedDependency, Arbiter::Promise<ArbiterDependencyList>> _dependencyListFetches;

    Arbiter::Future<ArbiterDependencyList> insertDependencyListFetch (Arbiter::Resolver::ResolvedDependency fetch);
    Arbiter::Promise<ArbiterDependencyList> extractDependencyListFetch (const Arbiter::Resolver::ResolvedDependency &fetch);

    // C exports
    static void dependencyListFetchOnSuccess (ArbiterDependencyListFetch cFetch, const ArbiterDependencyList *fetchedList);
    static void dependencyListFetchOnError (ArbiterDependencyListFetch cFetch);
    static void availableVersionsFetchOnNext (ArbiterAvailableVersionsFetch cFetch, const ArbiterSelectedVersion *nextVersion);
    static void availableVersionsFetchOnCompleted (ArbiterAvailableVersionsFetch cFetch);
    static void availableVersionsFetchOnError (ArbiterAvailableVersionsFetch cFetch);
};
