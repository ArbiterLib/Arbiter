#include "Resolver.h"

#include <exception>
#include <stdexcept>
#include <thread>

using namespace Arbiter;
using namespace Resolver;

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
  return resolver->resolvedAll();
}

void ArbiterStartResolvingNextDependency (ArbiterResolver *resolver, ArbiterResolverCallbacks callbacks)
{
  auto future = resolver->resolveNext();

  std::thread([resolver, future = std::move(future), callbacks = std::move(callbacks)]() mutable {
    try {
      ResolvedDependency dependency = future.get();
      callbacks.onSuccess(resolver, &dependency.projectIdentifier, &dependency.selectedVersion);
    } catch (const std::exception &ex) {
      callbacks.onError(resolver, ex.what());
    }
  }).detach();
}

void ArbiterFreeResolver (ArbiterResolver *resolver)
{
  delete resolver;
}

ResolvedDependency ResolvedDependency::takeOwnership (ArbiterDependencyListFetch fetch) noexcept
{
  std::unique_ptr<ArbiterProjectIdentifier> project(const_cast<ArbiterProjectIdentifier *>(fetch.project));
  std::unique_ptr<ArbiterSelectedVersion> selectedVersion(const_cast<ArbiterSelectedVersion *>(fetch.selectedVersion));

  return ResolvedDependency { std::move(*project), std::move(*selectedVersion) };
}

bool ResolvedDependency::operator== (const ResolvedDependency &other) const noexcept
{
  return projectIdentifier == other.projectIdentifier && selectedVersion == other.selectedVersion;
}

bool ArbiterResolver::resolvedAll () const noexcept
{
  return _remainingDependencies._dependencies.empty();
}

std::future<ResolvedDependency> resolveNext ()
{
  std::promise<ResolvedDependency> promise;

  // TODO: Actually resolve
  
  return promise.get_future();
}

std::future<ArbiterDependencyList> ArbiterResolver::insertDependencyListFetch (ResolvedDependency dependency)
{
  std::lock_guard<std::mutex> guard(_fetchesMutex);

  return _dependencyListFetches[std::move(dependency)].get_future();
}

std::promise<ArbiterDependencyList> ArbiterResolver::extractDependencyListFetch (const ResolvedDependency &dependency)
{
  std::lock_guard<std::mutex> guard(_fetchesMutex);

  std::promise<ArbiterDependencyList> promise = std::move(_dependencyListFetches.at(dependency));
  _dependencyListFetches.erase(dependency);

  return promise;
}

std::future<ArbiterDependencyList> ArbiterResolver::fetchDependencyList (ResolvedDependency dependency)
{
  // Eventually freed in the C callback function.
  auto project = std::make_unique<ArbiterProjectIdentifier>(dependency.projectIdentifier);
  auto version = std::make_unique<ArbiterSelectedVersion>(dependency.selectedVersion);

  auto future = insertDependencyListFetch(std::move(dependency));

  _behaviors.fetchDependencyList(
    ArbiterDependencyListFetch { this, project.release(), version.release() },
    &dependencyListFetchOnSuccess,
    &dependencyListFetchOnError
  );

  return future;
}

void ArbiterResolver::dependencyListFetchOnSuccess (ArbiterDependencyListFetch cFetch, const ArbiterDependencyList *fetchedList)
{
  auto &resolver = *const_cast<ArbiterResolver *>(cFetch.resolver);
  auto dependency = ResolvedDependency::takeOwnership(cFetch);

  resolver
    .extractDependencyListFetch(dependency)
    .set_value(*fetchedList);
}

void ArbiterResolver::dependencyListFetchOnError (ArbiterDependencyListFetch cFetch)
{
  auto &resolver = *const_cast<ArbiterResolver *>(cFetch.resolver);
  auto dependency = ResolvedDependency::takeOwnership(cFetch);

  resolver
    .extractDependencyListFetch(dependency)
    // TODO: Better error reporting
    .set_exception(std::make_exception_ptr(std::runtime_error("Dependency list fetch failed")));
}
