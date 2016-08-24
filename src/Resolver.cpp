#include "Resolver.h"

#include "Exception.h"
#include "Hash.h"
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

namespace {

struct GraphsForProjectGenerator final
{
  public:
    explicit GraphsForProjectGenerator (const ArbiterResolver &resolver, DependencyGraph baseGraph, Optional<DependencyGraph::Node> dependent, ArbiterDependency dependency)
      : _resolver(resolver)
      , _baseGraph(std::move(baseGraph))
      , _dependent(std::move(dependent))
      , _dependency(std::move(dependency))
    {}

    Optional<DependencyGraph> operator() ()
    {
      if (!_possibleVersions) {
        _possibleVersions = _resolver.availableVersionsSatisfying(_dependency._projectIdentifier, _dependency.requirement());
        _possibleVersionsIter = _possibleVersions->begin();
      }

      if (_possibleVersionsIter == _possibleVersions->end()) {
        return None();
      }

      const ArbiterSelectedVersion &selectedVersion = *_possibleVersionsIter;

      if (!_selectedVersionDependencies) {
        _selectedVersionDependencies = _resolver.fetchDependencies(_dependency._projectIdentifier, selectedVersion);
        _selectedVersionDependenciesIter = _selectedVersionDependencies->_dependencies.begin();
      }

      DependencyGraph graph = _baseGraph;
      graph.addNode(DependencyGraph::Node(_dependency._projectIdentifier, selectedVersion), _dependency.requirement(), _dependent);

      if (_selectedVersionDependenciesIter == _selectedVersionDependencies->_dependencies.end()) {
        // No dependencies to explore for this graph.
        _selectedVersionDependencies = None();
        ++_possibleVersionsIter;

        return makeOptional(std::move(graph));
      }

      // TODO
      assert(false);
    }

  private:
    const ArbiterResolver &_resolver;
    DependencyGraph _baseGraph;
    Optional<DependencyGraph::Node> _dependent;
    ArbiterDependency _dependency;

    Optional<std::vector<ArbiterSelectedVersion>> _possibleVersions;
    std::vector<ArbiterSelectedVersion>::const_iterator _possibleVersionsIter;

    Optional<ArbiterDependencyList> _selectedVersionDependencies;
    std::vector<ArbiterDependency>::const_iterator _selectedVersionDependenciesIter;
};

} // namespace

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

bool ArbiterResolvedAllDependencies (const ArbiterResolver *resolver)
{
  return resolver->resolvedAll();
}

ArbiterResolvedDependency *ArbiterCreateNextResolvedDependency (ArbiterResolver *resolver, char **error)
{
  Optional<ArbiterResolvedDependency> dependency;

  try {
    dependency = resolver->resolveNext();
  } catch (const std::exception &ex) {
    if (error) {
      *error = copyCString(ex.what()).release();
    }

    return nullptr;
  }

  return new ArbiterResolvedDependency(std::move(*dependency));
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

bool ArbiterResolver::resolvedAll () const noexcept
{
  return _remainingDependencies._dependencies.empty();
}

ArbiterResolvedDependency resolveNext() noexcept(false)
{
  assert(false);
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

Generator<DependencyGraph> ArbiterResolver::possibleGraphsSatisfying (DependencyGraph baseGraph, Optional<DependencyGraph::Node> dependent, ArbiterProjectIdentifier project, const ArbiterRequirement &requirement) const
{
  return makeGenerator(GraphsForProjectGenerator(*this, std::move(baseGraph), std::move(dependent), ArbiterDependency(std::move(project), requirement)));
}

Generator<DependencyGraph> ArbiterResolver::possibleGraphsForDependencyList (const DependencyGraph &baseGraph, const ArbiterDependencyList &list) const
{
  std::unordered_map<ArbiterDependency, std::vector<ArbiterSelectedVersion>> validVersionsByDependency;

  for (const auto &dep : list._dependencies) {
    validVersionsByDependency[dep] = availableVersionsSatisfying(dep._projectIdentifier, dep.requirement());
  }

  return makeGenerator([list = std::move(list), it = list._dependencies.begin()] {
    return makeOptional(DependencyGraph());
  });
}
