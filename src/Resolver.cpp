#include "Resolver.h"

#include "Requirement.h"

#include <cassert>
#include <exception>
#include <map>
#include <set>
#include <stdexcept>

using namespace Arbiter;
using namespace Resolver;

namespace {

struct DependencyNode final
{
  public:
    const ArbiterProjectIdentifier _project;
    const ArbiterSelectedVersion _proposedVersion;

    std::unique_ptr<ArbiterRequirement> _requirement;
    std::set<DependencyNode> _dependencies;

    DependencyNode (ArbiterProjectIdentifier project, ArbiterSelectedVersion proposedVersion, const ArbiterRequirement &requirement)
      : _project(std::move(project))
      , _proposedVersion(std::move(proposedVersion))
      , _requirement(requirement.clone())
    {
      assert(_requirement->satisfiedBy(_proposedVersion._semanticVersion));
    }

    bool operator== (const DependencyNode &other) const
    {
      return _project == other._project && _proposedVersion == other._proposedVersion;
    }

    bool operator< (const DependencyNode &other) const
    {
      if (_project != other._project) {
        // There's no well-defined ordering between nodes for different
        // projects, and we don't want them to appear equal within a set.
        return true;
      }

      // Sort such that higher versions are tried first.
      return _proposedVersion > other._proposedVersion;
    }
};

class DependencyGraph final
{
  public:
    std::set<DependencyNode> _allNodes;
    std::map<DependencyNode, std::set<DependencyNode>> _edges;
    std::set<DependencyNode> _roots;

    bool operator== (const DependencyGraph &other) const
    {
      return _edges == other._edges && _roots == other._roots;
    }
};

} // namespace

ArbiterResolver *ArbiterCreateResolver (ArbiterResolverBehaviors behaviors, const ArbiterDependencyList *dependencyList, ArbiterUserValue context)
{
  return new ArbiterResolver(std::move(behaviors), *dependencyList, ArbiterResolver::Context(std::move(context)));
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
  resolver->resolveNext().add_callback([resolver, callbacks = std::move(callbacks)](const auto &result) {
    try {
      const ResolvedDependency &dependency = result.rightOrThrowLeft();
      callbacks.onSuccess(resolver, &dependency.projectIdentifier, &dependency.selectedVersion);
    } catch (const std::exception &ex) {
      callbacks.onError(resolver, ex.what());
    }
  });
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

Arbiter::Future<ResolvedDependency> resolveNext ()
{
  Arbiter::Promise<ResolvedDependency> promise;

  // TODO: Actually resolve
  
  return promise.get_future();
}

Arbiter::Future<ArbiterDependencyList> ArbiterResolver::insertDependencyListFetch (ResolvedDependency dependency)
{
  std::lock_guard<std::mutex> guard(_fetchesMutex);

  return _dependencyListFetches[std::move(dependency)].get_future();
}

Arbiter::Promise<ArbiterDependencyList> ArbiterResolver::extractDependencyListFetch (const ResolvedDependency &dependency)
{
  std::lock_guard<std::mutex> guard(_fetchesMutex);

  Arbiter::Promise<ArbiterDependencyList> promise = std::move(_dependencyListFetches.at(dependency));
  _dependencyListFetches.erase(dependency);

  return promise;
}

Arbiter::Future<ArbiterDependencyList> ArbiterResolver::fetchDependencyList (ResolvedDependency dependency)
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
