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

bool ArbiterResolvedDependencyGraphAddNode (ArbiterResolvedDependencyGraph *graph, const ArbiterResolvedDependency *node, char **error)
{
  try {
    graph->addNode(*node);
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
    *(buffer++) = new ArbiterResolvedDependency(pair.first, pair.second.version());
  }
}

const ArbiterSelectedVersion *ArbiterResolvedDependencyGraphProjectVersion (const ArbiterResolvedDependencyGraph *graph, const ArbiterProjectIdentifier *project)
{
  auto it = graph->nodes().find(*project);
  if (it == graph->nodes().end()) {
    return nullptr;
  }

  return &it->second.version();
}

size_t ArbiterResolvedDependencyGraphCountDependencies (const ArbiterResolvedDependencyGraph *graph, const ArbiterProjectIdentifier *project)
{
  auto it = graph->nodes().find(*project);
  if (it == graph->nodes().end()) {
    return 0;
  }

  return it->second._dependencies.size();
}

void ArbiterResolvedDependencyGraphGetAllDependencies (const ArbiterResolvedDependencyGraph *graph, const ArbiterProjectIdentifier *project, const ArbiterProjectIdentifier **buffer)
{
  auto it = graph->nodes().find(*project);
  if (it == graph->nodes().end()) {
    return;
  }

  for (const ArbiterProjectIdentifier &dependency : it->second._dependencies) {
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

bool ArbiterResolvedDependencyGraph::Node::operator== (const Node &other) const
{
  return _version == other._version && _dependencies == other._dependencies;
}

void ArbiterResolvedDependencyGraph::addNode (const ArbiterResolvedDependency &node) noexcept(false)
{
  const auto it = _nodes.find(node._project);
  if (it == _nodes.end()) {
    _nodes.emplace(std::make_pair(node._project, Node(node._version)));
  } else {
    const ArbiterSelectedVersion &existingVersion = it->second.version();
    if (existingVersion != node._version) {
      throw Exception::ConflictingNode("Project " + toString(node._project) + " already exists in graph with version " + toString(existingVersion) + ", cannot add version " + toString(node._version));
    }
  }
}

void ArbiterResolvedDependencyGraph::addEdge (const ArbiterProjectIdentifier &dependent, ArbiterProjectIdentifier dependency)
{
  _nodes.at(dependent)._dependencies.emplace(std::move(dependency));
}

ArbiterResolvedDependencyGraph ArbiterResolvedDependencyGraph::graphWithNewRoots (const std::vector<ArbiterProjectIdentifier> &roots) const
{
  ArbiterResolvedDependencyGraph graph;

  for (const ArbiterProjectIdentifier &root : roots) {
    walkNodeAndCopyInto(graph, root, None());
  }

  return graph;
}

void ArbiterResolvedDependencyGraph::walkNodeAndCopyInto (ArbiterResolvedDependencyGraph &newGraph, const ArbiterProjectIdentifier &key, const Arbiter::Optional<ArbiterProjectIdentifier> &dependent) const
{
  const Node &node = _nodes.at(key);

  newGraph.addNode(ArbiterResolvedDependency(key, node.version()));
  if (dependent) {
    newGraph.addEdge(*dependent, key);
  }

  for (const ArbiterProjectIdentifier &dependency : node._dependencies) {
    walkNodeAndCopyInto(newGraph, dependency, makeOptional(key));
  }
}

ArbiterResolvedDependency ArbiterResolvedDependencyGraph::resolveNode (const ArbiterProjectIdentifier &projectIdentifier) const
{
  return ArbiterResolvedDependency(projectIdentifier, _nodes.at(projectIdentifier).version());
}

ArbiterResolvedDependencyInstaller ArbiterResolvedDependencyGraph::createInstaller () const
{
  ArbiterResolvedDependencyInstaller installer;

  if (_nodes.empty()) {
    return installer;
  }

  // Contains edges which still need to be added to the resolved graph.
  std::map<ArbiterProjectIdentifier, std::set<ArbiterProjectIdentifier>> remainingEdges;

  // Contains dependencies without any dependencies themselves.
  ArbiterResolvedDependencyInstaller::PhaseSet leaves;

  for (const auto &pair : _nodes) {
    const ArbiterProjectIdentifier &key = pair.first;
    const Node &node = pair.second;

    const auto &dependencySet = node._dependencies;
    if (dependencySet.empty()) {
      leaves.emplace(ArbiterResolvedDependency(key, node.version()));
      continue;
    }

    remainingEdges[key] = dependencySet;

    std::vector<ArbiterProjectIdentifier> dependencyList(dependencySet.begin(), dependencySet.end());
    assert(std::is_sorted(dependencyList.begin(), dependencyList.end()));

    installer._edges.emplace(std::make_pair(key, std::move(dependencyList)));
  }

  assert(installer._edges.size() + leaves.size() == _nodes.size());
  installer._phases.emplace_back(std::move(leaves));

  while (!remainingEdges.empty()) {
    ArbiterResolvedDependencyInstaller::PhaseSet thisPhase;

    for (auto edgeIt = remainingEdges.begin(); edgeIt != remainingEdges.end(); ) {
      const ArbiterProjectIdentifier &dependent = edgeIt->first;
      auto &dependencies = edgeIt->second;

      for (auto depIt = dependencies.begin(); depIt != dependencies.end(); ) {
        const ArbiterProjectIdentifier &dependency = *depIt;

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
  os << "Nodes:";
  for (const auto &pair : _nodes) {
    os << "\n\t" << resolveNode(pair.first) << " ->";

    for (const ArbiterProjectIdentifier &dependency : pair.second._dependencies) {
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
  
  return _nodes == ptr->_nodes;
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
