#include "Resolver.h"

#include "Hash.h"
#include "Requirement.h"

#include <cassert>
#include <exception>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

using namespace Arbiter;
using namespace Resolver;

namespace {

/**
 * Filters out versions from the given generator which fail the given
 * requirement, saving whatever is left (even if just a terminal event) into
 * `promise`.
 */
void satisfyPromiseWithFirstVersionPassingRequirement (std::shared_ptr<Promise<Optional<ArbiterSelectedVersion>>> promise, std::shared_ptr<Generator<ArbiterSelectedVersion>> versions, std::shared_ptr<ArbiterRequirement> requirement)
{
  versions->next().add_callback([promise, versions, requirement](const auto &result) {
    if (result.hasLeft() || !result.right() || requirement->satisfiedBy(result.right()->_semanticVersion)) {
      promise->set_result(result);
    } else {
      satisfyPromiseWithFirstVersionPassingRequirement(promise, versions, requirement);
    }
  });
}

/**
 * A node in, or being considered for, an acyclic dependency graph.
 */
class DependencyNode final
{
  public:
    struct Hash final
    {
      public:
        size_t operator() (const DependencyNode &node) const
        {
          return hashOf(node._project);
        }
    };

    const ArbiterProjectIdentifier _project;

    /**
     * The version of the dependency that this node represents.
     *
     * This version is merely "proposed" because it depends on the final
     * resolution of the graph, as well as whether any "better" graphs exist.
     */
    const ArbiterSelectedVersion _proposedVersion;

    DependencyNode (ArbiterProjectIdentifier project, ArbiterSelectedVersion proposedVersion, const ArbiterRequirement &requirement)
      : _project(std::move(project))
      , _proposedVersion(std::move(proposedVersion))
      , _state(std::make_shared<State>())
    {
      setRequirement(requirement);
    }

    DependencyNode (const DependencyNode &other) noexcept
      : _project(other._project)
      , _proposedVersion(other._proposedVersion)
      , _state(other._state)
    {}

    DependencyNode (DependencyNode &&other) noexcept
      : _project(other._project)
      , _proposedVersion(other._proposedVersion)
      , _state(std::move(other._state))
    {}

    DependencyNode &operator= (const DependencyNode &) = delete;
    DependencyNode &operator= (DependencyNode &&) = delete;

    bool operator== (const DependencyNode &other) const
    {
      // For the purposes of node lookup in a graph, this is the only field
      // which matters.
      return _project == other._project;
    }

    /**
     * The current requirement(s) applied to this dependency.
     *
     * This specifier may change as the graph is added to, and the requirements
     * become more stringent.
     */
    const ArbiterRequirement &requirement () const noexcept
    {
      return *_state->_requirement;
    }

    void setRequirement (const ArbiterRequirement &requirement)
    {
      setRequirement(requirement.clone());
    }

    void setRequirement (std::unique_ptr<ArbiterRequirement> requirement)
    {
      assert(requirement);
      assert(requirement->satisfiedBy(_proposedVersion._semanticVersion));

      _state->_requirement = std::move(requirement); 
    }

    std::unordered_set<DependencyNode, Hash> &dependencies () noexcept
    {
      return _state->_dependencies;
    }

    const std::unordered_set<DependencyNode, Hash> &dependencies () const noexcept
    {
      return _state->_dependencies;
    }

  private:
    struct State final
    {
      public:
        std::unique_ptr<ArbiterRequirement> _requirement;
        std::unordered_set<DependencyNode, Hash> _dependencies;
    };

    std::shared_ptr<State> _state;
};

std::ostream &operator<< (std::ostream &os, const DependencyNode &node)
{
  return os
    << node._project << " @ " << node._proposedVersion
    << " (restricted to " << node.requirement() << ")";
}

} // namespace

namespace std {

template<>
struct hash<DependencyNode> final
{
  public:
    size_t operator() (const DependencyNode &node) const
    {
      return DependencyNode::Hash()(node);
    }
};

} // namespace std

namespace {

/**
 * Represents an acyclic dependency graph in which each project appears at most
 * once.
 *
 * Dependency graphs can exist in an incomplete state, but will never be
 * inconsistent (i.e., include versions that are known to be invalid given the
 * current graph).
 */
class DependencyGraph final
{
  public:
    /**
     * A full list of all nodes included in the graph.
     */
    std::unordered_set<DependencyNode> _allNodes;

    /**
     * Maps between nodes in the graph and their immediate dependencies.
     */
    std::unordered_map<DependencyNode, std::unordered_set<DependencyNode>> _edges;

    /**
     * The root nodes of the graph (i.e., those dependencies that are listed by
     * the top-level project).
     */
    std::unordered_set<DependencyNode> _roots;

    bool operator== (const DependencyGraph &other) const
    {
      return _edges == other._edges && _roots == other._roots;
    }

    /**
     * Attempts to add the given node into the graph, as a dependency of
     * `dependent` if specified.
     *
     * If the given node refers to a project which already exists in the graph,
     * this method will attempt to intersect the version requirements of both.
     *
     * Returns a pointer to the node as actually inserted into the graph (which
     * may be different from the node passed in), or `nullptr` if this addition
     * would make the graph inconsistent.
     */
    DependencyNode *addNode (const DependencyNode &inputNode, const DependencyNode *dependent)
    {
      auto nodeInsertion = _allNodes.insert(inputNode);

      // Unordered collections rightly discourage mutation so hashes don't get
      // invalidated, but we've already handled this in the implementation of
      // DependencyNode.
      DependencyNode &insertedNode = const_cast<DependencyNode &>(*nodeInsertion.first);

      // If no insertion was actually performed, we need to unify our input with
      // what was already there.
      if (!nodeInsertion.second) {
        if (auto newRequirement = insertedNode.requirement().intersect(inputNode.requirement())) {
          if (!newRequirement->satisfiedBy(insertedNode._proposedVersion._semanticVersion)) {
            // This strengthened requirement invalidates the version we've
            // proposed in this graph, so the graph would become inconsistent.
            return nullptr;
          }

          insertedNode.setRequirement(std::move(newRequirement));
        } else {
          // If intersecting the requirements is impossible, the versions
          // currently shouldn't be able to match.
          //
          // Notably, though, Carthage does permit scenarios like this when
          // pinned to a branch. Arbiter doesn't support requirements like this
          // right now, but may in the future.
          assert(inputNode._proposedVersion != insertedNode._proposedVersion);
          return nullptr;
        }
      }

      if (dependent) {
        _edges[*dependent].insert(insertedNode);
      } else {
        _roots.insert(insertedNode);
      }

      return &insertedNode;
    }
};

std::ostream &operator<< (std::ostream &os, const DependencyGraph &graph)
{
  os << "Roots:";
  for (const DependencyNode &root : graph._roots) {
    os << "\n\t" << root;
  }

  os << "\n\nEdges";
  for (const auto &pair : graph._edges) {
    const DependencyNode &node = pair.first;
    os << "\n\t" << node._project << " ->";

    const auto &dependencies = pair.second;
    for (const DependencyNode &dep : dependencies) {
      os << "\n\t\t" << dep;
    }
  }

  return os;
}

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
  std::lock_guard<std::mutex> lock(_fetchesMutex);

  return _dependencyListFetches[std::move(dependency)].get_future();
}

Arbiter::Promise<ArbiterDependencyList> ArbiterResolver::extractDependencyListFetch (const ResolvedDependency &dependency)
{
  std::lock_guard<std::mutex> lock(_fetchesMutex);

  Arbiter::Promise<ArbiterDependencyList> promise = std::move(_dependencyListFetches.at(dependency));
  _dependencyListFetches.erase(dependency);

  return promise;
}

Arbiter::Generator<ArbiterSelectedVersion> ArbiterResolver::insertAvailableVersionsFetch (ArbiterProjectIdentifier fetch)
{
  std::lock_guard<std::mutex> lock(_fetchesMutex);

  return _availableVersionsFetches[std::move(fetch)].getGenerator();
}

Arbiter::Sink<ArbiterSelectedVersion> ArbiterResolver::extractAvailableVersionsFetch (const ArbiterProjectIdentifier &fetch)
{
  std::lock_guard<std::mutex> lock(_fetchesMutex);

  Arbiter::Sink<ArbiterSelectedVersion> sink = std::move(_availableVersionsFetches.at(fetch));
  _availableVersionsFetches.erase(fetch);

  return sink;
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

Arbiter::Generator<ArbiterSelectedVersion> ArbiterResolver::fetchAvailableVersions (ArbiterProjectIdentifier project)
{
  // Eventually freed in the C callback function.
  auto projectCopy = std::make_unique<ArbiterProjectIdentifier>(project);

  auto generator = insertAvailableVersionsFetch(std::move(project));

  _behaviors.fetchAvailableVersions(
    ArbiterAvailableVersionsFetch { this, projectCopy.release() },
    &availableVersionsFetchOnNext,
    &availableVersionsFetchOnCompleted,
    &availableVersionsFetchOnError
  );

  return generator;
}

Generator<ArbiterSelectedVersion> ArbiterResolver::fetchAvailableVersions (ArbiterProjectIdentifier project, const ArbiterRequirement &requirement)
{
  auto allVersions = std::make_shared<Generator<ArbiterSelectedVersion>>(fetchAvailableVersions(project));
  std::shared_ptr<ArbiterRequirement> sharedRequirement(requirement.clone());

  return Arbiter::Generator<ArbiterSelectedVersion>([allVersions, sharedRequirement] {
    auto promise = std::make_shared<Promise<Optional<ArbiterSelectedVersion>>>();

    satisfyPromiseWithFirstVersionPassingRequirement(promise, allVersions, sharedRequirement);

    return promise->get_future();
  });
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

void ArbiterResolver::availableVersionsFetchOnNext (ArbiterAvailableVersionsFetch cFetch, const ArbiterSelectedVersion *nextVersion)
{
  auto &resolver = *const_cast<ArbiterResolver *>(cFetch.resolver);
  const auto &project = *cFetch.project;

  std::lock_guard<std::mutex> lock(resolver._fetchesMutex);
  resolver
    ._availableVersionsFetches
    .at(project)
    .onNext(*nextVersion);
}

void ArbiterResolver::availableVersionsFetchOnCompleted (ArbiterAvailableVersionsFetch cFetch)
{
  auto &resolver = *const_cast<ArbiterResolver *>(cFetch.resolver);
  std::unique_ptr<ArbiterProjectIdentifier> project(const_cast<ArbiterProjectIdentifier *>(cFetch.project));

  resolver
    .extractAvailableVersionsFetch(*project)
    .onCompleted();
}

void ArbiterResolver::availableVersionsFetchOnError (ArbiterAvailableVersionsFetch cFetch)
{
  auto &resolver = *const_cast<ArbiterResolver *>(cFetch.resolver);
  std::unique_ptr<ArbiterProjectIdentifier> project(const_cast<ArbiterProjectIdentifier *>(cFetch.project));

  resolver
    .extractAvailableVersionsFetch(*project)
    // TODO: Better error reporting
    .onError(std::make_exception_ptr(std::runtime_error("Available versions fetch failed")));
}
