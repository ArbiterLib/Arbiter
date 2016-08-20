#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include "Dependency.h"
#include "Future.h"
#include "Generator.h"
#include "Hash.h"
#include "Value.h"
#include "Version.h"

#include <arbiter/Resolver.h>

#include <mutex>
#include <unordered_map>

namespace Arbiter {
namespace Resolver {

/**
 * Represents a selected version of a dependency.
 */
struct ResolvedDependency final
{
  ArbiterProjectIdentifier projectIdentifier;
  ArbiterSelectedVersion selectedVersion;

  /**
   * Given a fetch request from the C API, creates a ResolvedDependency
   * corresponding to the dependency whose transitive dependencies were being
   * looked up.
   */
  static ResolvedDependency takeOwnership (ArbiterDependencyListFetch fetch) noexcept;

  bool operator== (const ResolvedDependency &other) const noexcept;
};

} // namespace Resolver
} // namespace Arbiter

namespace std {

template<>
struct hash<Arbiter::Resolver::ResolvedDependency> final
{
  public:
    size_t operator() (const Arbiter::Resolver::ResolvedDependency &fetch) const
    {
      return Arbiter::hashOf(fetch.projectIdentifier)
        ^ Arbiter::hashOf(fetch.selectedVersion);
    }
};

} // namespace std

struct ArbiterResolver final
{
  public:
    using Context = Arbiter::SharedUserValue<ArbiterResolver>;

    Context _context;

    ArbiterResolver (ArbiterResolverBehaviors behaviors, ArbiterDependencyList dependencyList, Context context)
      : _context(std::move(context))
      , _behaviors(std::move(behaviors))
      , _remainingDependencies(std::move(dependencyList))
    {}

    /**
     * Fetches the list of dependencies for the given project and version.
     */
    Arbiter::Future<ArbiterDependencyList> fetchDependencyList (Arbiter::Resolver::ResolvedDependency fetch);

    /**
     * Fetches the list of available versions for the given project.
     */
    Arbiter::Generator<ArbiterSelectedVersion> fetchAvailableVersions (ArbiterProjectIdentifier project);

    /**
     * Returns whether all dependencies have been resolved.
     */
    bool resolvedAll () const noexcept;

    /**
     * Begins resolving the next unresolved dependency.
     *
     * After starting to resolve a dependency, the resolver must not be
     * destroyed until the future has been satisfied.
     *
     * It is invalid to invoke this method if resolvedAll() returns true.
     */
    Arbiter::Future<Arbiter::Resolver::ResolvedDependency> resolveNext ();

  private:
    const ArbiterResolverBehaviors _behaviors;
    ArbiterDependencyList _remainingDependencies;

    std::mutex _fetchesMutex;
    std::unordered_map<Arbiter::Resolver::ResolvedDependency, Arbiter::Promise<ArbiterDependencyList>> _dependencyListFetches;
    std::unordered_map<ArbiterProjectIdentifier, Arbiter::Sink<ArbiterSelectedVersion>> _availableVersionsFetches;

    /**
     * Creates a promise/future pair representing a fetch for the given
     * dependency, and returns the "read" end of it.
     */
    Arbiter::Future<ArbiterDependencyList> insertDependencyListFetch (Arbiter::Resolver::ResolvedDependency fetch);

    /**
     * Given an in-flight fetch, returns the "write" end where the result should
     * be placed, and removes it from the collection of in-flight fetches.
     */
    Arbiter::Promise<ArbiterDependencyList> extractDependencyListFetch (const Arbiter::Resolver::ResolvedDependency &fetch);

    /**
     * Creates a generator/sink pair representing a fetch for available versions
     * of the given project, and returns the "read" end of it.
     */
    Arbiter::Generator<ArbiterSelectedVersion> insertAvailableVersionsFetch (ArbiterProjectIdentifier fetch);

    /**
     * Given an in-flight fetch, returns the "write" end where the terminal
     * event should be sent, and removes it from the collection of in-flight
     * fetches.
     */
    Arbiter::Sink<ArbiterSelectedVersion> extractAvailableVersionsFetch (const ArbiterProjectIdentifier &fetch);

    // C exports
    static void dependencyListFetchOnSuccess (ArbiterDependencyListFetch cFetch, const ArbiterDependencyList *fetchedList);
    static void dependencyListFetchOnError (ArbiterDependencyListFetch cFetch);
    static void availableVersionsFetchOnNext (ArbiterAvailableVersionsFetch cFetch, const ArbiterSelectedVersion *nextVersion);
    static void availableVersionsFetchOnCompleted (ArbiterAvailableVersionsFetch cFetch);
    static void availableVersionsFetchOnError (ArbiterAvailableVersionsFetch cFetch);
};
