#include "Resolver.h"

#include "Algorithm.h"
#include "Exception.h"
#include "Optional.h"
#include "Requirement.h"
#include "Stats.h"
#include "ToString.h"

#include <algorithm>
#include <cassert>
#include <exception>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <stack>
#include <unordered_set>

using namespace Arbiter;

namespace {

struct Node final
{
  public:
    using Dependencies = std::unordered_set<ArbiterProjectIdentifier>;

    Node (std::unique_ptr<ArbiterRequirement> requirement, std::shared_ptr<Arbiter::Instantiation> instantiation)
      : _requirement(std::move(requirement))
      , _instantiation(std::move(instantiation))
    {}

    Node (const Node &other)
      : _requirement(other.requirement().cloneRequirement())
      , _dependencies(other._dependencies)
      , _instantiation(other._instantiation)
    {}

    Node &operator= (const Node &other)
    {
      if (this == &other) {
        return *this;
      }

      _requirement = other.requirement().cloneRequirement();
      _dependencies = other._dependencies;
      _instantiation = other._instantiation;
      return *this;
    }

    const ArbiterRequirement &requirement () const
    {
      return *_requirement;
    }

    void setRequirement (std::unique_ptr<ArbiterRequirement> newRequirement)
    {
      assert(_instantiation->satisfies(*newRequirement));
      _requirement = std::move(newRequirement);
    }

    const std::shared_ptr<Instantiation> &instantiation () const
    {
      return _instantiation;
    }

    void setInstantiation (std::shared_ptr<Instantiation> newInstantiation)
    {
      assert(newInstantiation->satisfies(*_requirement));
      _instantiation = std::move(newInstantiation);
    }

    const Dependencies &dependencies () const
    {
      return _dependencies;
    }

    void addDependency (const ArbiterProjectIdentifier &projectIdentifier)
    {
      _dependencies.emplace(projectIdentifier);
    }

  private:
    std::unique_ptr<ArbiterRequirement> _requirement;
    Dependencies _dependencies;
    std::shared_ptr<Instantiation> _instantiation;
};

struct Graph final
{
  public:
    using Nodes = std::unordered_map<ArbiterProjectIdentifier, Node>;

    std::shared_ptr<Instantiation> addNode (ArbiterResolver &resolver, const ArbiterProjectIdentifier &projectIdentifier, const ArbiterRequirement &requirement) noexcept(false)
    {
      auto it = _nodes.find(projectIdentifier);
      if (it == _nodes.end()) {
        std::shared_ptr<Instantiation> instantiation = resolver.bestProjectInstantiationSatisfying(projectIdentifier, requirement);

        if (!instantiation) {
          throw Exception::UnsatisfiableConstraints("Cannot satisfy " + toString(requirement) + " from available versions of " + toString(projectIdentifier));
        }

        _nodes.emplace(std::make_pair(projectIdentifier, Node(requirement.cloneRequirement(), instantiation)));
        return instantiation;
      }

      Node &node = it->second;
      std::unique_ptr<ArbiterRequirement> intersection = node.requirement().intersect(requirement);
      if (!intersection) {
        // TODO: Track more information about where each requirement came from.
        throw Exception::MutuallyExclusiveConstraints(toString(requirement) + " and " + toString(node.requirement()) + " on " + toString(projectIdentifier) + " are mutually exclusive");
      }

      if (!node.instantiation()->satisfies(*intersection)) {
        // TODO: Exclude this instantiation in the most recent dependent's
        // requirement for this project, to avoid trying it again if we
        // backtrack.
        throw Exception::UnsatisfiableConstraints("Cannot satisfy " + toString(*intersection) + " on " + toString(projectIdentifier) + " with " + toString(*node.instantiation()));

        // TODO: Pick a new instantiation, then diff the dependencies of the two
        // different instantiations? This would make success more likely along
        // the current path, but it complicates backtracking (to where we could
        // end up doing duplicate work if not careful).
        #if 0
        std::shared_ptr<Instantiation> newInstantiation = resolver.bestProjectInstantiationSatisfying(projectIdentifier, intersection);
        if (!newInstantiation) {
          throw Exception::UnsatisfiableConstraints("Cannot satisfy " + toString(*intersection) + " with " + toString(node.instantiation()));
        }

        // This should always succeed, even with the old requirement in place,
        // because we're only about to make it stricter.
        node.setInstantiation(std::move(newInstantiation));
        #endif
      }

      node.setRequirement(std::move(intersection));
      return node.instantiation();
    }

    void addEdge (const ArbiterProjectIdentifier &dependent, const ArbiterProjectIdentifier &dependency)
    {
      _nodes.at(dependent).addDependency(dependency);
    }

    const Nodes &nodes () const
    {
      return _nodes;
    }

    ArbiterResolvedDependency resolveProject (const ArbiterProjectIdentifier &projectIdentifier) const
    {
      const Node &node = _nodes.at(projectIdentifier);

      // TODO: Move into definition of Instantiation?
      const auto &versions = node.instantiation()->_versions;
      auto it = std::find_if(versions.begin(), versions.end(), [&](const ArbiterSelectedVersion &version) {
        return node.requirement().satisfiedBy(version);
      });

      // We must find a version if the node's instantiation satisfied its
      // requirement.
      assert(it != versions.end());

      return ArbiterResolvedDependency(projectIdentifier, *it);
    }

    explicit operator ArbiterResolvedDependencyGraph () const
    {
      // TODO: Pare down ArbiterResolvedDependencyGraph so this conversion is
      // cheaper.
      ArbiterResolvedDependencyGraph converted;
      for (const auto &pair : nodes()) {
        converted.addNode(resolveProject(pair.first), pair.second.requirement());
        for (const ArbiterProjectIdentifier &dependency : pair.second.dependencies()) {
          converted.addEdge(pair.first, dependency);
        }
      }
      
      return converted;
    }

  private:
    Nodes _nodes;
};

std::ostream &operator<< (std::ostream &os, const Graph &graph)
{
  os << "Graph:";

  for (const auto &pair : graph.nodes()) {
    os << "\n" << pair.first << " " << *pair.second.instantiation() << " (constrained to " << pair.second.requirement() << ")";
  }

  return os;
}

Graph resolveDependencies (ArbiterResolver &resolver, const Graph &baseGraph, Optional<ArbiterProjectIdentifier> dependent, std::unordered_set<ArbiterDependency> dependencySet) noexcept(false)
{
  std::map<ArbiterProjectIdentifier, std::unique_ptr<ArbiterRequirement>> requirementsByProject;
  for (const ArbiterDependency &dependency : dependencySet) {
    requirementsByProject.emplace(std::make_pair(dependency._projectIdentifier, dependency.requirement().cloneRequirement()));
  }

  reset(dependencySet);

retry:
  Graph graph = baseGraph;
  for (auto it = requirementsByProject.begin(); it != requirementsByProject.end(); ) {
    const ArbiterProjectIdentifier &projectIdentifier = it->first;

    auto instantiation = graph.addNode(resolver, projectIdentifier, *it->second);
    if (dependent) {
      graph.addEdge(*dependent, projectIdentifier);
    }

    try {
      Graph newGraph = resolveDependencies(resolver, graph, makeOptional(projectIdentifier), instantiation->dependencies());
      graph = std::move(newGraph);
    } catch (Exception::Base &) {
      if (std::unique_ptr<ArbiterRequirement> intersection = Requirement::ExcludedInstantiation(instantiation).intersect(*it->second)) {
        it->second = std::move(intersection);
        goto retry;
      }

      throw;
    }

    ++it;
  }

  return graph;
}

} // namespace

ArbiterResolver *ArbiterCreateResolver (ArbiterResolverBehaviors behaviors, const struct ArbiterResolvedDependencyGraph *initialGraph, const struct ArbiterDependencyList *dependenciesToResolve, ArbiterUserContext context)
{
  return new ArbiterResolver(std::move(behaviors), (initialGraph ? *initialGraph : ArbiterResolvedDependencyGraph()), *dependenciesToResolve, shareUserContext(context));
}

const void *ArbiterResolverContext (const ArbiterResolver *resolver)
{
  return resolver->_context.get();
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

const Arbiter::Instantiation::Dependencies &ArbiterResolver::fetchDependencies (const ArbiterProjectIdentifier &projectIdentifier, const ArbiterSelectedVersion &version) noexcept(false)
{
  // This project should already be present, as its domain must have been known
  // to obtain `version`.
  Project &project = _projects.at(projectIdentifier);

  if (auto inst = project.instantiationForVersion(version)) {
    return inst->dependencies();
  }

  char *error = nullptr;
  std::unique_ptr<ArbiterDependencyList> dependencyList(_behaviors.createDependencyList(this, &projectIdentifier, &version, &error));

  ++_latestStats._dependencyListFetches;

  if (dependencyList) {
    assert(!error);

    return project.addInstantiation(version, std::move(*dependencyList))->dependencies();
  } else if (error) {
    throw Exception::UserError(copyAcquireCString(error));
  } else {
    throw Exception::UserError();
  }
}

const Arbiter::Project::Domain &ArbiterResolver::fetchAvailableVersions (const ArbiterProjectIdentifier &projectIdentifier) noexcept(false)
{
  auto it = _projects.find(projectIdentifier);
  if (it == _projects.end()) {
    char *error = nullptr;
    std::unique_ptr<ArbiterSelectedVersionList> versionList(_behaviors.createAvailableVersionsList(this, &projectIdentifier, &error));

    ++_latestStats._availableVersionFetches;

    if (!versionList) {
      if (error) {
        throw Exception::UserError(copyAcquireCString(error));
      } else {
        throw Exception::UserError();
      }
    }

    assert(!error);

    Project::Domain domain(std::make_move_iterator(versionList->_versions.begin()), std::make_move_iterator(versionList->_versions.end()));
    it = _projects.emplace(std::make_pair(projectIdentifier, Project(std::move(domain)))).first;
  }

  return it->second.domain();
}

Optional<ArbiterSelectedVersion> ArbiterResolver::fetchSelectedVersionForMetadata (const ArbiterProjectIdentifier &project, const Arbiter::SharedUserValue<ArbiterSelectedVersion> &metadata)
{
  const auto behavior = _behaviors.createSelectedVersionForMetadata;
  if (!behavior) {
    return None();
  }

  std::unique_ptr<ArbiterSelectedVersion> version(behavior(this, &project, metadata.data()));
  if (version) {
    return makeOptional(std::move(*version));
  } else {
    return None();
  }
}

ArbiterResolvedDependencyGraph ArbiterResolver::resolve () noexcept(false)
{
  std::deque<std::pair<ArbiterProjectIdentifier, std::unique_ptr<ArbiterRequirement>>> variables;
  std::deque<std::shared_ptr<Arbiter::Instantiation>> values;

  for (const ArbiterDependency &dependency : _dependenciesToResolve._dependencies) {
    variables.emplace_back(std::make_pair(dependency._projectIdentifier, dependency.requirement().cloneRequirement()));
  }

  while (values.size() < variables.size()) {
    try {
      const auto &variable = variables.at(values.size());
      const ArbiterProjectIdentifier &projectIdentifier = variable.first;
      const ArbiterRequirement &requirement = *variable.second;

      std::shared_ptr<Instantiation> instantiationPtr = bestProjectInstantiationSatisfying(projectIdentifier, requirement);
      if (!instantiationPtr) {
        throw Exception::UnsatisfiableConstraints("Cannot satisfy " + toString(requirement) + " from available versions of " + toString(projectIdentifier));
      }

      const Instantiation &instantiation = *instantiationPtr;
      values.emplace_back(std::move(instantiationPtr));

      for (const ArbiterDependency &dependency : instantiation.dependencies()) {
        auto it = std::find_if(variables.begin(), variables.end(), [&](const auto &pair) {
          return pair.first == dependency._projectIdentifier;
        });

        if (it == variables.end()) {
          variables.emplace_back(std::make_pair(dependency._projectIdentifier, dependency.requirement().cloneRequirement()));
          continue;
        }

        std::unique_ptr<ArbiterRequirement> intersection = it->second->intersect(dependency.requirement());
        if (!intersection) {
          throw Exception::MutuallyExclusiveConstraints(toString(dependency.requirement()) + " and " + toString(*it->second) + " on " + toString(dependency._projectIdentifier) + " are mutually exclusive");
        }

        const ArbiterRequirement &newRequirement = *intersection;

        // TODO: Don't replace with intersection, instead add this dependency as
        // a new variable, or do something else that permits us to remove this
        // requirement when backtracking
        it->second = std::move(intersection);

        size_t index = it - variables.begin();
        if (index < values.size()) {
          const Instantiation &instantiation = *values.at(index);
          if (!instantiation.satisfies(newRequirement)) {
            throw Exception::UnsatisfiableConstraints("Cannot satisfy " + toString(newRequirement) + " on " + toString(dependency._projectIdentifier) + " with " + toString(instantiation));
          }
        }
      }
    } catch (Arbiter::Exception::Base &ex) {
      if (values.size() == 0) {
        // Nothing further to try.
        throw;
      }

      size_t index = values.size() - 1;

      std::shared_ptr<Arbiter::Instantiation> instantiation = values.at(index);
      values.pop_back();

      Requirement::ExcludedInstantiation newRequirement(std::move(instantiation));
      auto &variable = variables.at(index);

      std::unique_ptr<ArbiterRequirement> intersection = variable.second->intersect(newRequirement);
      assert(intersection);

      // TODO: Don't replace with intersection, instead add this dependency as
      // a new variable, or do something else that permits us to remove this
      // requirement when backtracking
      variable.second = std::move(intersection);
    }
  }

  ArbiterResolvedDependencyGraph graph;
  for (size_t i = 0; i < variables.size(); ++i) {
    const auto &variable = variables.at(i);
    const Instantiation &value = *values.at(i);

    Optional<ArbiterSelectedVersion> selectedVersion = value.bestVersionSatisfying(*variable.second);

    // We must find a version if the node's instantiation satisfied its
    // requirement.
    assert(selectedVersion);

    graph.addNode(ArbiterResolvedDependency(variable.first, std::move(*selectedVersion)), *variable.second);
    for (const ArbiterDependency &dependency : value.dependencies()) {
      graph.addEdge(variable.first, dependency._projectIdentifier);
    }
  }

  // TODO: Stats collection
  return graph;
}

std::unique_ptr<Arbiter::Base> ArbiterResolver::clone () const
{
  return std::make_unique<ArbiterResolver>(_behaviors, _initialGraph, _dependenciesToResolve, _context);
}

std::ostream &ArbiterResolver::describe (std::ostream &os) const
{
  return os << "ArbiterResolver: " << _dependenciesToResolve;
}

bool ArbiterResolver::operator== (const Arbiter::Base &other) const
{
  return this == &other;
}

std::shared_ptr<Arbiter::Instantiation> ArbiterResolver::bestProjectInstantiationSatisfying (const ArbiterProjectIdentifier &projectIdentifier, const ArbiterRequirement &requirement)
{
  const auto &availableVersions = fetchAvailableVersions(projectIdentifier);
  const auto it = std::find_if(availableVersions.begin(), availableVersions.end(), [&](const ArbiterSelectedVersion &version) {
    return requirement.satisfiedBy(version);
  });

  if (it == availableVersions.end()) {
    return nullptr;
  }

  const ArbiterSelectedVersion &bestVersion = *it;

  // There should be an entry in _projects due to fetchAvailableVersions()
  // above.
  Project &project = _projects.at(projectIdentifier);

  // FIXME: This is a hack to populate the instantiation we want.
  fetchDependencies(projectIdentifier, bestVersion);
  auto inst = project.instantiationForVersion(bestVersion);
  assert(inst);
  return inst;
}

void ArbiterResolver::startStats ()
{
  _latestStats = Stats(Stats::Clock::now());
}

void ArbiterResolver::endStats ()
{
  _latestStats._endTime = Stats::Clock::now();

  size_t depsSize = 0;
  size_t versionsSize = _projects.size() * sizeof(decltype(_projects)::key_type);

  // These size estimates are a holdover from a previous representation, where
  // we cached simple maps of project identifiers to versions and dependency
  // sets. Do our best to maintain compatibility with those measurements.
  for (const auto &pair : _projects) {
    const Project &project = pair.second;
    versionsSize += project.domain().size() * sizeof(Project::Domain::value_type);

    for (const std::shared_ptr<Instantiation> &instantiation : project.instantiations()) {
      depsSize += instantiation->dependencies().size() * sizeof(Instantiation::Dependencies::value_type);
      depsSize += instantiation->_versions.size() * sizeof(Instantiation::Versions::value_type);
    }
  }

  _latestStats._cachedDependenciesSizeEstimate = depsSize;
  _latestStats._cachedAvailableVersionsSizeEstimate = versionsSize;
}
