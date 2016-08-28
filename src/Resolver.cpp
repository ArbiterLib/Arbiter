#include "Resolver.h"

#include "Algorithm.h"
#include "Exception.h"
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

void DependencyGraph::addNode (ArbiterResolvedDependency node, const ArbiterRequirement &initialRequirement, const Optional<ArbiterProjectIdentifier> &dependent) noexcept(false)
{
  assert(initialRequirement.satisfiedBy(node._version._semanticVersion));

  const NodeKey &key = node._project;

  const auto it = _nodeMap.find(key);
  if (it != _nodeMap.end()) {
    NodeValue &value = it->second;

    // We need to unify our input with what was already there.
    if (auto newRequirement = initialRequirement.intersect(value.requirement())) {
      if (!newRequirement->satisfiedBy(value._version._semanticVersion)) {
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

std::vector<ArbiterResolvedDependency> DependencyGraph::allNodes() const
{
  std::vector<ArbiterResolvedDependency> nodes;
  nodes.reserve(_nodeMap.size());

  for (const auto &pair : _nodeMap) {
    nodes.emplace_back(resolveNode(pair.first, pair.second));
  }

  return nodes;
}

DependencyGraph::NodeValue::NodeValue (ArbiterSelectedVersion version, const ArbiterRequirement &requirement)
  : _version(std::move(version))
{
  setRequirement(requirement);
}

const ArbiterRequirement &DependencyGraph::NodeValue::requirement () const
{
  return *_requirement;
}

void DependencyGraph::NodeValue::setRequirement (const ArbiterRequirement &requirement)
{
  setRequirement(requirement.clone());
}

void DependencyGraph::NodeValue::setRequirement (std::unique_ptr<ArbiterRequirement> requirement)
{
  assert(requirement->satisfiedBy(_version._semanticVersion));
  _requirement = std::move(requirement);
}

ArbiterResolvedDependency DependencyGraph::resolveNode (const NodeKey &key, const NodeValue &value)
{
  return ArbiterResolvedDependency(key, value._version);
}

ArbiterResolvedDependency DependencyGraph::resolveNode (const NodeKey &key) const
{
  return resolveNode(key, _nodeMap.at(key));
}

std::ostream &DependencyGraph::describe (std::ostream &os) const
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

std::ostream &operator<< (std::ostream &os, const DependencyGraph &graph)
{
  return graph.describe(os);
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
  // TODO: Replace _dependencyList with this.
  std::set<ArbiterDependency> dependencySet(_dependencyList._dependencies.begin(), _dependencyList._dependencies.end());

  DependencyGraph graph = resolveDependencies(DependencyGraph(), std::move(dependencySet), std::unordered_map<ArbiterProjectIdentifier, ArbiterProjectIdentifier>());
  return ArbiterResolvedDependencyList(graph.allNodes());
}

DependencyGraph ArbiterResolver::resolveDependencies (const DependencyGraph &baseGraph, const std::set<ArbiterDependency> &dependencySet, const std::unordered_map<ArbiterProjectIdentifier, ArbiterProjectIdentifier> &dependentsByProject) noexcept(false)
{
  if (dependencySet.empty()) {
    return baseGraph;
  }

  std::map<ArbiterProjectIdentifier, std::unique_ptr<ArbiterRequirement>> requirementsByProject;

  for (const ArbiterDependency &dependency : dependencySet) {
    requirementsByProject[dependency._projectIdentifier] = dependency.requirement().clone();
  }

  assert(requirementsByProject.size() == dependencySet.size());

  std::map<ArbiterProjectIdentifier, std::vector<ArbiterResolvedDependency>> possibilities;

  for (const auto &pair : requirementsByProject) {
    const ArbiterProjectIdentifier &project = pair.first;
    const ArbiterRequirement &requirement = *pair.second;

    std::vector<ArbiterSelectedVersion> versions = availableVersionsSatisfying(project, requirement);
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
        std::vector<ArbiterDependency> transitives = fetchDependencies(dependency._project, dependency._version)._dependencies;

        for (const ArbiterDependency &transitive : transitives) {
          dependentsByTransitive[transitive._projectIdentifier] = dependency._project;
        }

        collectedTransitives.insert(transitives.begin(), transitives.end());
      }

      // TODO: Free local collections before recursing?

      return resolveDependencies(candidate, std::move(collectedTransitives), std::move(dependentsByTransitive));
    } catch (Arbiter::Exception::Base &ex) {
      lastException = std::current_exception();
    }
  }

  rethrow_exception(lastException);
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
