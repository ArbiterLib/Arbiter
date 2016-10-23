#include "Graph.h"

#include <algorithm>

#include "Exception.h"

using namespace Arbiter;

ArbiterResolvedDependencyGraph *ArbiterResolvedDependencyGraphCreate (void)
{
  return new ArbiterResolvedDependencyGraph;
}

ArbiterResolvedDependencyGraph *ArbiterResolvedDependencyGraphCopyWithNewRoots (const ArbiterResolvedDependencyGraph *baseGraph, const struct ArbiterProjectIdentifier * const *roots, size_t rootCount)
{
  std::vector<ArbiterProjectIdentifier> vec;
  vec.reserve(rootCount);

  for (size_t i = 0; i < rootCount; i++) {
    vec.emplace_back(*roots[i]);
  }

  return new ArbiterResolvedDependencyGraph(baseGraph->graphWithNewRoots(std::move(vec)));
}

bool ArbiterResolvedDependencyGraphAddNode (ArbiterResolvedDependencyGraph *graph, const ArbiterResolvedDependency *node, const ArbiterRequirement *requirement, char **error)
{
  try {
    graph->addNode(*node, *requirement);
    return true;
  } catch (const Exception::Base &ex) {
    if (error) {
      *error = copyCString(ex.what()).release();
    }

    return false;
  }
}

bool ArbiterResolvedDependencyGraphAddEdge (ArbiterResolvedDependencyGraph *graph, const ArbiterProjectIdentifier *dependent, const ArbiterProjectIdentifier *dependency, char **error)
{
  try {
    graph->addEdge(*dependent, *dependency);
    return true;
  } catch (const Exception::Base &ex) {
    if (error) {
      *error = copyCString(ex.what()).release();
    }

    return false;
  }
}

size_t ArbiterResolvedDependencyGraphCount (const ArbiterResolvedDependencyGraph *graph)
{
  return graph->nodes().size();
}

void ArbiterResolvedDependencyGraphCopyAll (const ArbiterResolvedDependencyGraph *graph, ArbiterResolvedDependency **buffer)
{
  for (const auto &pair : graph->nodes()) {
    *(buffer++) = new ArbiterResolvedDependency(ArbiterResolvedDependencyGraph::resolveNode(pair));
  }
}

const ArbiterSelectedVersion *ArbiterResolvedDependencyGraphProjectVersion (const ArbiterResolvedDependencyGraph *graph, const ArbiterProjectIdentifier *project)
{
  auto it = graph->nodes().find(*project);
  if (it == graph->nodes().end()) {
    return nullptr;
  }

  return &it->second._version;
}

const ArbiterRequirement *ArbiterResolvedDependencyGraphProjectRequirement (const ArbiterResolvedDependencyGraph *graph, const ArbiterProjectIdentifier *project)
{
  auto it = graph->nodes().find(*project);
  if (it == graph->nodes().end()) {
    return nullptr;
  }

  return &it->second.requirement();
}

size_t ArbiterResolvedDependencyGraphCountDependencies (const ArbiterResolvedDependencyGraph *graph, const ArbiterProjectIdentifier *project)
{
  auto it = graph->edges().find(*project);
  if (it == graph->edges().end()) {
    return 0;
  }

  return it->second.size();
}

void ArbiterResolvedDependencyGraphGetAllDependencies (const ArbiterResolvedDependencyGraph *graph, const ArbiterProjectIdentifier *project, const ArbiterProjectIdentifier **buffer)
{
  auto it = graph->edges().find(*project);
  if (it == graph->edges().end()) {
    return;
  }

  for (const ArbiterProjectIdentifier &dependency : it->second) {
    *(buffer++) = &dependency;
  }
}

ArbiterResolvedDependencyInstaller *ArbiterResolvedDependencyInstallerCreate (const ArbiterResolvedDependencyGraph *graph)
{
  return new ArbiterResolvedDependencyInstaller(graph->createInstaller());
}

size_t ArbiterResolvedDependencyInstallerPhaseCount (const ArbiterResolvedDependencyInstaller *installer)
{
  return installer->_phases.size();
}

size_t ArbiterResolvedDependencyInstallerCountInPhase (const ArbiterResolvedDependencyInstaller *installer, size_t phaseIndex)
{
  return installer->countInPhase(phaseIndex);
}

void ArbiterResolvedDependencyInstallerGetAllInPhase (const ArbiterResolvedDependencyInstaller *installer, size_t phaseIndex, const ArbiterResolvedDependency **buffer)
{
  const auto &phase = installer->_phases.at(phaseIndex);
  for (const ArbiterResolvedDependency &dependency : phase) {
    *(buffer++) = &dependency;
  }
}

ArbiterResolvedDependencyGraph::NodeValue::NodeValue (ArbiterSelectedVersion version, const ArbiterRequirement &requirement)
  : _version(std::move(version))
{
  setRequirement(requirement);
}

bool ArbiterResolvedDependencyGraph::NodeValue::operator== (const NodeValue &other) const
{
  return _version == other._version && requirement() == other.requirement();
}

void ArbiterResolvedDependencyGraph::NodeValue::setRequirement (const ArbiterRequirement &requirement)
{
  setRequirement(requirement.cloneRequirement());
}

void ArbiterResolvedDependencyGraph::NodeValue::setRequirement (std::unique_ptr<ArbiterRequirement> requirement)
{
  assert(requirement->satisfiedBy(_version));
  _requirement = std::move(requirement);
}

void ArbiterResolvedDependencyGraph::addNode (ArbiterResolvedDependency node, const ArbiterRequirement &initialRequirement) noexcept(false)
{
  assert(initialRequirement.satisfiedBy(node._version));

  const NodeKey &key = node._project;

  const auto it = _nodes.find(key);
  if (it != _nodes.end()) {
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
    _nodes.emplace(std::make_pair(key, NodeValue(node._version, initialRequirement)));
  }
}

void ArbiterResolvedDependencyGraph::addEdge (const ArbiterProjectIdentifier &dependent, ArbiterProjectIdentifier dependency)
{
  assert(_nodes.find(dependent) != _nodes.end());

  _edges[dependent].emplace(std::move(dependency));
}

ArbiterResolvedDependencyGraph ArbiterResolvedDependencyGraph::graphWithNewRoots (const std::vector<NodeKey> &roots) const
{
  ArbiterResolvedDependencyGraph graph;

  for (const NodeKey &root : roots) {
    walkNodeAndCopyInto(graph, root, None());
  }

  return graph;
}

void ArbiterResolvedDependencyGraph::walkNodeAndCopyInto (ArbiterResolvedDependencyGraph &newGraph, const NodeKey &key, const Arbiter::Optional<NodeKey> &dependent) const
{
  newGraph.addNode(resolveNode(key), _nodes.at(key).requirement());
  if (dependent) {
    newGraph.addEdge(*dependent, key);
  }

  const auto it = _edges.find(key);
  if (it == _edges.end()) {
    return;
  }

  const auto &dependencies = it->second;
  for (const NodeKey &dependency : dependencies) {
    walkNodeAndCopyInto(newGraph, dependency, makeOptional(key));
  }
}

ArbiterResolvedDependency ArbiterResolvedDependencyGraph::resolveNode (const NodeMap::value_type &node)
{
  return ArbiterResolvedDependency(node.first, node.second._version);
}

ArbiterResolvedDependency ArbiterResolvedDependencyGraph::resolveNode (const NodeMap::key_type &key) const
{
  return resolveNode(std::make_pair(key, _nodes.at(key)));
}

ArbiterResolvedDependencyInstaller ArbiterResolvedDependencyGraph::createInstaller () const
{
  ArbiterResolvedDependencyInstaller installer;
  if (_nodes.empty()) {
    return installer;
  }

  // Contains edges which still need to be added to the resolved graph.
  EdgeMap remainingEdges;
  remainingEdges.reserve(_edges.size());

  // Contains dependencies without any dependencies themselves.
  ArbiterResolvedDependencyInstaller::PhaseSet leaves;

  for (const auto &pair : _nodes) {
    const NodeKey &key = pair.first;
    const auto it = _edges.find(key);

    if (it == _edges.end()) {
      leaves.emplace(resolveNode(key));
    } else {
      const auto &dependencySet = it->second;

      remainingEdges[key] = dependencySet;

      std::vector<NodeKey> dependencyList(dependencySet.begin(), dependencySet.end());
      std::sort(dependencyList.begin(), dependencyList.end());

      installer._edges.emplace(std::make_pair(key, std::move(dependencyList)));
    }
  }

  assert(installer._edges.size() == _edges.size());
  installer._phases.emplace_back(std::move(leaves));

  while (!remainingEdges.empty()) {
    ArbiterResolvedDependencyInstaller::PhaseSet thisPhase;

    for (auto edgeIt = remainingEdges.begin(); edgeIt != remainingEdges.end(); ) {
      const NodeKey &dependent = edgeIt->first;
      auto &dependencies = edgeIt->second;

      for (auto depIt = dependencies.begin(); depIt != dependencies.end(); ) {
        const NodeKey &dependency = *depIt;

        // If this dependency is in the graph already, it can be removed
        // from the list of remaining edges.
        if (installer.contains(resolveNode(dependency))) {
          depIt = dependencies.erase(depIt);
        } else {
          ++depIt;
        }
      }

      // If all dependencies are now in the graph, we can add this node to
      // the current phase we're building.
      if (dependencies.empty()) {
        thisPhase.emplace(resolveNode(dependent));
        edgeIt = remainingEdges.erase(edgeIt);
      } else {
        ++edgeIt;
      }
    }

    installer._phases.emplace_back(std::move(thisPhase));
  }

  return installer;
}

std::unique_ptr<Arbiter::Base> ArbiterResolvedDependencyGraph::clone () const
{
  return std::make_unique<ArbiterResolvedDependencyGraph>(*this);
}

std::ostream &ArbiterResolvedDependencyGraph::describe (std::ostream &os) const
{
  os << "Roots:";
  for (const auto &pair : _nodes) {
    const NodeKey &key = pair.first;
    if (_edges.find(key) == _edges.end()) {
      os << "\n\t" << resolveNode(pair);
    }
  }

  os << "\n\nEdges:";
  for (const auto &pair : _edges) {
    os << "\n\t" << resolveNode(pair.first) << " ->";

    for (const NodeKey &dependency : pair.second) {
      os << "\n\t\t" << resolveNode(dependency);
    }
  }

  return os;
}

bool ArbiterResolvedDependencyGraph::operator== (const Arbiter::Base &other) const
{
  auto ptr = dynamic_cast<const ArbiterResolvedDependencyGraph *>(&other);
  if (!ptr) {
    return false;
  }
  
  return _nodes == ptr->_nodes && _edges == ptr->_edges;
}

size_t ArbiterResolvedDependencyInstaller::countInPhase (size_t phaseIndex) const
{
  return _phases.at(phaseIndex).size();
}

bool ArbiterResolvedDependencyInstaller::contains (const ArbiterResolvedDependency &node) const
{
  for (const PhaseSet &phase : _phases) {
    if (phase.find(node) != phase.end()) {
      return true;
    }
  }

  return false;
}

std::unique_ptr<Arbiter::Base> ArbiterResolvedDependencyInstaller::clone () const
{
  return std::make_unique<ArbiterResolvedDependencyInstaller>(*this);
}

std::ostream &ArbiterResolvedDependencyInstaller::describe (std::ostream &os) const
{
  os << "Install-ordered graph:";

  for (const auto &phase : _phases) {
    for (const auto &dependency : phase) {
      os << "\n" << dependency;
    }
  }

  return os;
}

bool ArbiterResolvedDependencyInstaller::operator== (const Arbiter::Base &other) const
{
  auto ptr = dynamic_cast<const ArbiterResolvedDependencyInstaller *>(&other);
  if (!ptr) {
    return false;
  }

  return _edges == ptr->_edges && _phases == ptr->_phases;
}
