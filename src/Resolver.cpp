#include "Resolver.h"

#include "Algorithm.h"
#include "Exception.h"
#include "Iterator.h"
#include "Optional.h"
#include "Requirement.h"
#include "ToString.h"

#include <cassert>
#include <exception>
#include <map>
#include <set>
#include <unordered_set>

using namespace Arbiter;

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
     * Attempts to add the given node into the graph, as a dependency of
     * `dependent` if specified.
     *
     * If the given node refers to a project which already exists in the graph,
     * this method will attempt to intersect the version requirements of both.
     *
     * Throws an exception if this addition would make the graph inconsistent.
     */
    void addNode (ArbiterResolvedDependency node, const ArbiterRequirement &initialRequirement, const Optional<ArbiterProjectIdentifier> &dependent) noexcept(false)
    {
      assert(initialRequirement.satisfiedBy(node._version));

      const NodeKey &key = node._project;

      const auto it = _nodeMap.find(key);
      if (it != _nodeMap.end()) {
        NodeValue &value = it->second;

        // We need to unify our input with what was already there.
        if (auto newRequirement = initialRequirement.intersect(value.requirement())) {
          if (!newRequirement->satisfiedBy(value._version)) {
            throw Exception::UnsatisfiableConstraints("Cannot satisfy " + toString(*newRequirement) + " with " + toString(value._version));
          }

          value.setRequirement(std::move(newRequirement));
        } else {
          throw Exception::MutuallyExclusiveConstraints(toString(value.requirement()) + " and " + toString(initialRequirement) + " are mutually exclusive");
        }
      } else {
        _nodeMap.emplace(std::make_pair(key, NodeValue(node._version, initialRequirement)));
      }

      if (dependent) {
        _edges[*dependent].insert(key);
      } else {
        _roots.insert(key);
      }
    }

    ArbiterResolvedDependencyGraph resolvedGraph () const
    {
      ArbiterResolvedDependencyGraph resolved;
      if (_nodeMap.empty()) {
        return resolved;
      }

      // Contains edges which still need to be added to the resolved graph.
      std::unordered_map<NodeKey, std::unordered_set<NodeKey>> remainingEdges;

      // Contains dependencies without any dependencies themselves.
      ArbiterResolvedDependencyGraph::DepthSet leaves;

      for (const auto &pair : _nodeMap) {
        const NodeKey &key = pair.first;
        const auto it = _edges.find(key);

        if (it == _edges.end()) {
          leaves.emplace(resolveNode(key));
        } else {
          remainingEdges[key] = it->second;
        }
      }

      resolved._depths.emplace_back(std::move(leaves));

      while (!remainingEdges.empty()) {
        ArbiterResolvedDependencyGraph::DepthSet thisDepth;

        for (auto edgeIt = remainingEdges.begin(); edgeIt != remainingEdges.end(); ) {
          const NodeKey &dependent = edgeIt->first;
          auto &dependencies = edgeIt->second;

          for (auto depIt = dependencies.begin(); depIt != dependencies.end(); ) {
            const NodeKey &dependency = *depIt;

            // If this dependency is in the graph already, it can be removed
            // from the list of remaining edges.
            if (resolved.contains(resolveNode(dependency))) {
              depIt = dependencies.erase(depIt);
            } else {
              ++depIt;
            }
          }

          // If all dependencies are now in the graph, we can add this node to
          // the current depth we're building.
          if (dependencies.empty()) {
            thisDepth.emplace(resolveNode(dependent));
            edgeIt = remainingEdges.erase(edgeIt);
          } else {
            ++edgeIt;
          }
        }

        resolved._depths.emplace_back(std::move(thisDepth));
      }

      assert(resolved.count() == _nodeMap.size());
      return resolved;
    }

    std::ostream &describe (std::ostream &os) const
    {
      os << "Roots:";
      for (const NodeKey &key : _roots) {
        os << "\n\t" << resolveNode(key);
      }

      os << "\n\nEdges";
      for (const auto &pair : _edges) {
        const NodeKey &key = pair.first;
        os << "\n\t" << key << " ->";

        for (const NodeKey &dependency : pair.second) {
          os << "\n\t\t" << resolveNode(dependency);
        }
      }

      return os;
    }

  private:
    using NodeKey = ArbiterProjectIdentifier;

    struct NodeValue final
    {
      public:
        const ArbiterSelectedVersion _version;

        NodeValue (ArbiterSelectedVersion version, const ArbiterRequirement &requirement)
          : _version(std::move(version))
        {
          setRequirement(requirement);
        }

        const ArbiterRequirement &requirement () const
        {
          return *_requirement;
        }

        void setRequirement (const ArbiterRequirement &requirement)
        {
          setRequirement(requirement.cloneRequirement());
        }

        void setRequirement (std::unique_ptr<ArbiterRequirement> requirement)
        {
          assert(requirement->satisfiedBy(_version));
          _requirement = std::move(requirement);
        }

      private:
        std::shared_ptr<ArbiterRequirement> _requirement;
    };

    static ArbiterResolvedDependency resolveNode (const NodeKey &key, const NodeValue &value)
    {
      return ArbiterResolvedDependency(key, value._version);
    }

    ArbiterResolvedDependency resolveNode (const NodeKey &key) const
    {
      return resolveNode(key, _nodeMap.at(key));
    }

    // TODO: Should these be unordered, with ordering instead applied in
    // resolvedGraph?
    std::set<NodeKey> _roots;
    // TODO: This should probably be a multimap.
    std::map<NodeKey, std::unordered_set<NodeKey>> _edges;
    std::unordered_map<NodeKey, NodeValue> _nodeMap;
};

DependencyGraph resolveDependencies (ArbiterResolver &resolver, const DependencyGraph &baseGraph, std::set<ArbiterDependency> dependencySet, const std::unordered_map<ArbiterProjectIdentifier, ArbiterProjectIdentifier> &dependentsByProject) noexcept(false)
{
  if (dependencySet.empty()) {
    return baseGraph;
  }

  // This collection is reused when actually building the new dependency graph
  // below.
  std::map<ArbiterProjectIdentifier, std::unique_ptr<ArbiterRequirement>> requirementsByProject;

  for (const ArbiterDependency &dependency : dependencySet) {
    requirementsByProject[dependency._projectIdentifier] = dependency.requirement().cloneRequirement();
  }

  assert(requirementsByProject.size() == dependencySet.size());

  // Free the dependencySet, as it will no longer be used.
  reset(dependencySet);

  // This collection needs to exist for as long as the permuted iterators do below.
  std::map<ArbiterProjectIdentifier, std::vector<ArbiterResolvedDependency>> possibilities;

  for (const auto &pair : requirementsByProject) {
    const ArbiterProjectIdentifier &project = pair.first;
    const ArbiterRequirement &requirement = *pair.second;

    std::vector<ArbiterSelectedVersion> versions = resolver.availableVersionsSatisfying(project, requirement);
    if (versions.empty()) {
      throw Exception::UnsatisfiableConstraints("Cannot satisfy " + toString(requirement) + " from available versions of " + toString(project));
    }

    // Sort the version list with highest precedence first, so we try the newest
    // possible versions first.
    std::sort(versions.begin(), versions.end(), std::greater<ArbiterSelectedVersion>());

    std::vector<ArbiterResolvedDependency> resolutions;
    resolutions.reserve(versions.size());

    for (ArbiterSelectedVersion &version : versions) {
      resolutions.emplace_back(project, std::move(version));
    }

    possibilities[project] = std::move(resolutions);
  }

  assert(possibilities.size() == requirementsByProject.size());

  using Iterator = std::vector<ArbiterResolvedDependency>::const_iterator;

  std::vector<IteratorRange<Iterator>> ranges;
  for (const auto &pair : possibilities) {
    const std::vector<ArbiterResolvedDependency> &dependencies = pair.second;
    ranges.emplace_back(dependencies.cbegin(), dependencies.cend());
  }

  assert(ranges.size() == possibilities.size());

  std::exception_ptr lastException = std::make_exception_ptr(Exception::UnsatisfiableConstraints("No further combinations to attempt"));

  for (PermutationIterator<Iterator> permuter(std::move(ranges)); permuter; ++permuter) {
    try {
      std::vector<ArbiterResolvedDependency> choices = *permuter;

      DependencyGraph candidate = baseGraph;

      // Add everything to the graph first, to throw any exceptions that would
      // occur before we perform the computation- and memory-expensive stuff for
      // transitive dependencies.
      for (ArbiterResolvedDependency &dependency : choices) {
        const ArbiterRequirement &requirement = *requirementsByProject.at(dependency._project);
        candidate.addNode(dependency, requirement, maybeAt(dependentsByProject, dependency._project));
      }

      // Collect immediate children for the next phase of dependency resolution,
      // so we can permute their versions as a group (for something
      // approximating breadth-first search).
      std::set<ArbiterDependency> collectedTransitives;
      std::unordered_map<ArbiterProjectIdentifier, ArbiterProjectIdentifier> dependentsByTransitive;

      for (ArbiterResolvedDependency &dependency : choices) {
        std::vector<ArbiterDependency> transitives = resolver.fetchDependencies(dependency._project, dependency._version)._dependencies;

        for (const ArbiterDependency &transitive : transitives) {
          dependentsByTransitive[transitive._projectIdentifier] = dependency._project;
        }

        collectedTransitives.insert(transitives.begin(), transitives.end());
      }

      reset(choices);

      return resolveDependencies(resolver, candidate, std::move(collectedTransitives), std::move(dependentsByTransitive));
    } catch (Arbiter::Exception::Base &ex) {
      lastException = std::current_exception();
    }
  }

  std::rethrow_exception(lastException);
}

} // namespace

ArbiterResolver *ArbiterCreateResolver (ArbiterResolverBehaviors behaviors, const ArbiterDependencyList *dependencyList, const void *context)
{
  return new ArbiterResolver(std::move(behaviors), *dependencyList, context);
}

const void *ArbiterResolverContext (const ArbiterResolver *resolver)
{
  return resolver->_context;
}

ArbiterResolvedDependencyGraph *ArbiterResolverCreateResolvedDependencyGraph (ArbiterResolver *resolver, char **error)
{
  Optional<ArbiterResolvedDependencyGraph> dependencies;

  try {
    dependencies = resolver->resolve();
  } catch (const std::exception &ex) {
    if (error) {
      *error = copyCString(ex.what()).release();
    }

    return nullptr;
  }

  return new ArbiterResolvedDependencyGraph(std::move(*dependencies));
}

void ArbiterFreeResolver (ArbiterResolver *resolver)
{
  delete resolver;
}

ArbiterDependencyList ArbiterResolver::fetchDependencies (const ArbiterProjectIdentifier &project, const ArbiterSelectedVersion &version) noexcept(false)
{
  ArbiterResolvedDependency resolved(project, version);
  if (auto list = maybeAt(_cachedDependencies, resolved)) {
    return *list;
  }

  char *error = nullptr;
  std::unique_ptr<ArbiterDependencyList> dependencyList(_behaviors.createDependencyList(this, &project, &version, &error));

  if (dependencyList) {
    assert(!error);

    _cachedDependencies[resolved] = *dependencyList;
    return std::move(*dependencyList);
  } else if (error) {
    throw Exception::UserError(copyAcquireCString(error));
  } else {
    throw Exception::UserError();
  }
}

ArbiterSelectedVersionList ArbiterResolver::fetchAvailableVersions (const ArbiterProjectIdentifier &project) noexcept(false)
{
  if (auto list = maybeAt(_cachedAvailableVersions, project)) {
    return *list;
  }

  char *error = nullptr;
  std::unique_ptr<ArbiterSelectedVersionList> versionList(_behaviors.createAvailableVersionsList(this, &project, &error));

  if (versionList) {
    assert(!error);

    _cachedAvailableVersions[project] = *versionList;
    return std::move(*versionList);
  } else if (error) {
    throw Exception::UserError(copyAcquireCString(error));
  } else {
    throw Exception::UserError();
  }
}

ArbiterResolvedDependencyGraph ArbiterResolver::resolve () noexcept(false)
{
  std::set<ArbiterDependency> dependencySet(_dependencyList._dependencies.begin(), _dependencyList._dependencies.end());

  DependencyGraph graph = resolveDependencies(*this, DependencyGraph(), std::move(dependencySet), std::unordered_map<ArbiterProjectIdentifier, ArbiterProjectIdentifier>());
  return graph.resolvedGraph();
}

std::unique_ptr<Arbiter::Base> ArbiterResolver::clone () const
{
  return std::make_unique<ArbiterResolver>(_behaviors, _dependencyList, _context);
}

std::ostream &ArbiterResolver::describe (std::ostream &os) const
{
  return os << "ArbiterResolver: " << _dependencyList;
}

bool ArbiterResolver::operator== (const Arbiter::Base &other) const
{
  return this == &other;
}

std::vector<ArbiterSelectedVersion> ArbiterResolver::availableVersionsSatisfying (const ArbiterProjectIdentifier &project, const ArbiterRequirement &requirement) noexcept(false)
{
  std::vector<ArbiterSelectedVersion> versions = fetchAvailableVersions(project)._versions;

  auto removeStart = std::remove_if(versions.begin(), versions.end(), [&requirement](const ArbiterSelectedVersion &version) {
    return !requirement.satisfiedBy(version);
  });

  versions.erase(removeStart, versions.end());
  return versions;
}
