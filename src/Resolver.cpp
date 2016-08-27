#include "Resolver.h"

#include "Exception.h"
#include "Hash.h"
#include "Iterator.h"
#include "Optional.h"
#include "Requirement.h"
#include "ToString.h"

#include <algorithm>
#include <cassert>
#include <exception>
#include <map>
#include <stdexcept>

using namespace Arbiter;
using namespace Resolver;

void DependencyGraph::addNode (Node node, const ArbiterRequirement &initialRequirement, const Optional<Node> &dependent) noexcept(false)
{
  assert(initialRequirement.satisfiedBy(node._version._semanticVersion));

  std::unique_ptr<ArbiterRequirement> requirementToInsert;
  if (const std::shared_ptr<ArbiterRequirement> &existingRequirement = _requirementsByNode[node]) {
    // We need to unify our input with what was already there.
    if (auto newRequirement = initialRequirement.intersect(*existingRequirement)) {
      if (!newRequirement->satisfiedBy(node._version._semanticVersion)) {
        throw Exception::UnsatisfiableConstraints("Cannot satisfy " + toString(*newRequirement) + " with " + toString(node._version));
      }

      requirementToInsert = std::move(newRequirement);
    } else {
      throw Exception::MutuallyExclusiveConstraints(toString(*existingRequirement) + " and " + toString(initialRequirement) + " are mutually exclusive");
    }
  } else {
    requirementToInsert = initialRequirement.clone();
  }

  assert(requirementToInsert);
  _requirementsByNode[node] = std::move(requirementToInsert);

  if (dependent) {
    _edges[*dependent].insert(node);
  } else {
    _roots.insert(node);
  }
}

void DependencyGraph::concatGraph (const DependencyGraph &other, const Optional<Node> &dependent) noexcept(false)
{
  for (const auto &node : other.roots()) {
    addNode(node, *other._requirementsByNode.at(node), dependent);
  }

  for (const auto &pair : other.edges()) {
    Optional<Node> node(pair.first);

    for (const auto &dependency : pair.second) {
      addNode(dependency, *other._requirementsByNode.at(dependency), node);
    }
  }
}

std::vector<DependencyGraph::Node> DependencyGraph::allNodes() const
{
  std::vector<Node> nodes;
  nodes.reserve(_requirementsByNode.size());

  for (const auto &pair : _requirementsByNode) {
    nodes.emplace_back(pair.first);
  }

  return nodes;
}

bool DependencyGraph::operator== (const DependencyGraph &other) const
{
  return _edges == other._edges && _roots == other._roots;
}

std::ostream &operator<< (std::ostream &os, const DependencyGraph &graph)
{
  os << "Roots:";
  for (const DependencyGraph::Node &root : graph.roots()) {
    os << "\n\t" << root;
  }

  os << "\n\nEdges";
  for (const auto &pair : graph.edges()) {
    const DependencyGraph::Node &node = pair.first;
    os << "\n\t" << node._project << " ->";

    for (const DependencyGraph::Node &dependency : pair.second) {
      os << "\n\t\t" << dependency;
    }
  }

  return os;
}

ArbiterResolver *ArbiterCreateResolver (ArbiterResolverBehaviors behaviors, const ArbiterDependencyList *dependencyList, ArbiterUserValue context)
{
  return new ArbiterResolver(std::move(behaviors), *dependencyList, ArbiterResolver::Context(std::move(context)));
}

const void *ArbiterResolverContext (const ArbiterResolver *resolver)
{
  return resolver->_context.data();
}

ArbiterResolvedDependencyList *ArbiterResolverCreateResolvedDependencyList (ArbiterResolver *resolver, char **error)
{
  Optional<ArbiterResolvedDependencyList> dependencies;

  try {
    dependencies = resolver->resolve();
  } catch (const std::exception &ex) {
    if (error) {
      *error = copyCString(ex.what()).release();
    }

    return nullptr;
  }

  return new ArbiterResolvedDependencyList(std::move(*dependencies));
}

void ArbiterFreeResolver (ArbiterResolver *resolver)
{
  delete resolver;
}

ArbiterDependencyList ArbiterResolver::fetchDependencies (const ArbiterProjectIdentifier &project, const ArbiterSelectedVersion &version) const noexcept(false)
{
  char *error = nullptr;
  std::unique_ptr<ArbiterDependencyList> dependencyList(_behaviors.createDependencyList(this, &project, &version, &error));

  if (dependencyList) {
    assert(!error);
    return *dependencyList;
  } else if (error) {
    throw Exception::UserError(copyAcquireCString(error));
  } else {
    throw Exception::UserError();
  }
}

ArbiterSelectedVersionList ArbiterResolver::fetchAvailableVersions (const ArbiterProjectIdentifier &project) const noexcept(false)
{
  char *error = nullptr;
  std::unique_ptr<ArbiterSelectedVersionList> versionList(_behaviors.createAvailableVersionsList(this, &project, &error));

  if (versionList) {
    assert(!error);
    return *versionList;
  } else if (error) {
    throw Exception::UserError(copyAcquireCString(error));
  } else {
    throw Exception::UserError();
  }
}

ArbiterResolvedDependencyList ArbiterResolver::resolve () noexcept(false)
{
  std::map<ArbiterDependency, std::vector<ArbiterSelectedVersion>> versionsByDependency;
  versionsByDependency.reserve(_dependencyList._dependencies.size());

  for (const ArbiterDependency &dependency : _dependencyList._dependencies) {
    auto versions = availableVersionsSatisfying(dependency._projectIdentifier, dependency.requirement());

    // Sort the version list with highest precedence first, so we try the newest
    // possible versions first.
    std::sort(versions.begin(), versions.end(), std::greater<ArbiterSelectedVersion>());

    versionsByDependency[dependency] = std::move(versions);
  }

  for (const auto &pair : versionsByDependency) {
    const ArbiterDependency &dependency = pair.first;
    const std::vector<ArbiterSelectedVersion> &versions = pair.second;
  }

  /*
  DependencyGraph candidate;

  // Populate the initial roots of the graph.
  for (const auto &pair : versionsByDependency) {
    const ArbiterDependency &dependency = pair.first;
    const std::vector<ArbiterSelectedVersion> &versions = pair.second;

    if (versions.empty()) {
      throw Exception::UnsatisfiableConstraints("Cannot satisfy " + toString(dependency.requirement()) + " from available versions");
    }

    try {
      candidate.addNode(DependencyGraph::Node(dependency._projectIdentifier, versions.front()), dependency.requirement(), None());
    } catch (std::exception &ex) {
      // This shouldn't happen, because there aren't yet other requirements in
      // the graph with which this node could conflict.
      throw std::logic_error("Exception thrown while adding dependency graph root: " + ex.what());
    }
  }
  */

  // TODO
  assert(false);
}

DependencyGraph ArbiterResolver::attemptRoots (const std::unordered_map<ArbiterDependency, std::vector<ArbiterSelectedVersion>> &versionsByDependency, size_t attempt = 0) noexcept(false)
{
  // TODO: This should use an ordered map or a different structure, so that
  // dependency resolution is deterministic.
  std::unordered_map<ArbiterDependency, std::vector<ArbiterSelectedVersion>::const_iterator> selectedVersionByDependency;
  selectedVersionByDependency.reserve(versionsByDependency.size());

  // Extract iterators for each version list.
  for (const auto &pair : versionsByDependency) {
    const ArbiterDependency &dependency = pair.first;
    const std::vector<ArbiterSelectedVersion> &versions = pair.second;

    if (versions.empty()) {
      throw Exception::UnsatisfiableConstraints("Cannot satisfy " + toString(dependency.requirement()) + " from available versions");
    }

    selectedVersionByDependency[dependency] = versions.begin();
  }

  // Perform a breadth-first search over the list of versions, by advancing each
  // one incrementally until we get to a combination we haven't tried before.
  size_t remainingOffset = attempt;
  while (remainingOffset > 0) {
    bool movedOne = false;

    for (auto &pair : selectedVersionByDependency) {
      const ArbiterDependency &dependency = pair.first;
      const std::vector<ArbiterSelectedVersion> &versions = versionsByDependency[dependency];

      auto &iter = pair.second;
      if (iter == versions.end()) {
        continue;
      }

      ++iter;
      movedOne = true;

      if (--remainingOffset == 0) {
        break;
      }
    }

    if (!movedOne) {
      throw Exception::UnsatisfiableConstraints("No further combinations to attempt");
    }
  }

  // Create a "base" dependency graph that we will use to attempt transitive
  // dependencies upon.
  DependencyGraph baseGraph;

  // Collect immediate children for each version we selected.
  std::unordered_map<ArbiterResolvedDependency, ArbiterDependencyList> childDependencies;

  for (const auto &pair : selectedVersionByDependency) {
    const ArbiterDependency &dependency = pair.first;
    const ArbiterSelectedVersion &version = *pair.second;

    ArbiterResolvedDependency node(dependency._projectIdentifier, version);

    try {
      baseGraph.addNode(node, dependency.requirement(), None());
    } catch (std::exception &ex) {
      // This shouldn't happen, because there aren't yet other requirements in
      // the graph with which this node could conflict.
      throw std::logic_error("Exception thrown while adding dependency graph root: " + ex.what());
    }

    childDependencies[std::move(node)] = fetchDependencies(dependency._projectIdentifier, version);
  }

  // TODO: Clear data structures that are now unused, to save memory?

  try {
    // attempt dependencies
  } catch (Arbiter::Exception::Base &ex) {
    try {
      // Make another attempt with a new graph.
      return attemptRoots(versionsByDependency, attempt + 1);
    } catch (Arbiter::Exception::Base &) {
      // If that also fails, return the original root cause.
      throw ex;
    }
  }
}

std::vector<ArbiterSelectedVersion> ArbiterResolver::availableVersionsSatisfying (const ArbiterProjectIdentifier &project, const ArbiterRequirement &requirement) const noexcept(false)
{
  std::vector<ArbiterSelectedVersion> versions = fetchAvailableVersions(project)._versions;

  auto removeStart = std::remove_if(versions.begin(), versions.end(), [&requirement](const ArbiterSelectedVersion &version) {
    return !requirement.satisfiedBy(version._semanticVersion);
  });

  versions.erase(removeStart, versions.end());
  return versions;
}
