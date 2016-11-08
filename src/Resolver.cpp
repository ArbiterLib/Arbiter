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
#include <list>
#include <iostream>
#include <map>
#include <set>
#include <stack>
#include <unordered_set>

using namespace Arbiter;

namespace {

/**
 * Represents a constraint in a network, with information about where it
 * originated.
 */
struct Constraint final
{
  public:
    Constraint (std::shared_ptr<ArbiterRequirement> requirement, Optional<ArbiterProjectIdentifier> applicator)
      : _requirement(std::move(requirement))
      , _applicator(std::move(applicator))
    {}

    std::shared_ptr<ArbiterRequirement> _requirement;
    Optional<ArbiterProjectIdentifier> _applicator;
};

/**
 * Represents a variable in partially-solved constraint network.
 */
struct Variable final
{
  public:
    Variable (ArbiterProjectIdentifier projectIdentifier, std::shared_ptr<ArbiterRequirement> startingRequirement, Optional<ArbiterProjectIdentifier> dependent)
      : _projectIdentifier(std::move(projectIdentifier))
    {
      assert(startingRequirement);

      _constraints.emplace_back(std::move(startingRequirement), std::move(dependent));
    }

    const ArbiterProjectIdentifier &projectIdentifier () const
    {
      return _projectIdentifier;
    }

    Requirement::Compound requirement () const
    {
      Requirement::Compound compound;
      for (const Constraint &constraint : _constraints) {
        compound._requirements.emplace_back(constraint._requirement);
      }

      return compound;
    }

    void addRequirement (std::shared_ptr<ArbiterRequirement> requirement, Optional<ArbiterProjectIdentifier> dependent) noexcept(false)
    {
      // TODO: We shouldn't need to allocate a new requirement to do this
      // check.
      if (!requirement->intersect(this->requirement())) {
        throw Exception::MutuallyExclusiveConstraints(toString(*requirement) + " and " + toString(this->requirement()) + " on " + toString(_projectIdentifier) + " are mutually exclusive");
      }

      _constraints.emplace_back(std::move(requirement), std::move(dependent));
    }

    /**
     * Adds a requirement to this variable that disallows the given
     * instantiation.
     *
     * This method may only be used when there is at least one constraint on the
     * variable already. (Otherwise, there would be guidance for selecting
     * a version in the first place.)
     */
    void excludeInstantiation (std::shared_ptr<Instantiation> instantiation)
    {
      assert(_constraints.size() > 0);

      // Attribute the excluded instantiation to the last project which added
      // a requirement here, because trying a new instantiation of _that_
      // project should invalidate any exclusions on _this_ project which were
      // added afterward.
      const Constraint &lastConstraint = _constraints.back();

      std::shared_ptr<ArbiterRequirement> requirement = std::make_shared<Requirement::ExcludedInstantiation>(std::move(instantiation));
      _constraints.emplace_back(std::move(requirement), lastConstraint._applicator);
    }

    /**
     * Removes requirements on this variable which originated from the given
     * project.
     *
     * Returns the remaining number of requirements on the variable. If zero, it
     * should be removed from the current list of variables.
     */
    size_t removeRequirementsFrom (const ArbiterProjectIdentifier &applicator)
    {
      for (auto it = _constraints.begin(); it != _constraints.end();) {
        if (it->_applicator == makeOptional(applicator)) {
          it = _constraints.erase(it);
        } else {
          ++it;
        }
      }

      return _constraints.size();
    }

  private:
    ArbiterProjectIdentifier _projectIdentifier;
    std::list<const Constraint> _constraints;
};

/**
 * Represents a constraint network that is being solved.
 */
class Network final
{
  public:
    bool solved () const
    {
      return _values.size() == _variables.size();
    }

    const Variable &nextUnsolvedVariable () const
    {
      assert(!solved());

      return _variables.at(_values.size());
    }

    /**
     * Attempts to backtrack, un-solving one or more variables.
     *
     * Returns false if unable to backtrack any further.
     */
    bool backtrack ()
    {
      assert(!_variables.empty());

      if (_values.size() == 0) {
        // Nothing further to try.
        return false;
      }

      size_t index = _values.size() - 1;

      std::shared_ptr<Arbiter::Instantiation> instantiation = _values.at(index);
      _values.pop_back();

      ArbiterProjectIdentifier culprit = _variables.at(index).projectIdentifier();
      removeRequirementsFrom(culprit);

      // Variables before the culprit variable should not have moved at all.
      Variable &variable = _variables.at(index);
      assert(variable.projectIdentifier() == culprit);

      variable.excludeInstantiation(std::move(instantiation));

      return true;
    }

    void solveNextVariable (std::shared_ptr<Instantiation> value)
    {
      assert(!solved());

      _values.emplace_back(std::move(value));
    }

    /**
     * Attempts to add the given dependency into the constraint network, as an
     * unsolved variable.
     *
     * If the dependency already exists in the network with an assigned value,
     * the respective requirements are intersected. If the assigned value does
     * not satisfy the intersection, an exception is thrown, and no change to
     * the network occurs.
     */
    void enqueueDependency (const ArbiterDependency &dependency, Optional<ArbiterProjectIdentifier> dependent) noexcept(false)
    {
      auto it = std::find_if(_variables.begin(), _variables.end(), [&](const Variable &variable) {
        return variable.projectIdentifier() == dependency._projectIdentifier;
      });

      if (it == _variables.end()) {
        _variables.emplace_back(dependency._projectIdentifier, dependency.requirement().cloneRequirement(), std::move(dependent));
        return;
      }

      size_t index = it - _variables.begin();
      if (index < _values.size()) {
        const Instantiation &instantiation = *_values.at(index);
        if (!instantiation.satisfies(dependency.requirement())) {
          throw Exception::UnsatisfiableConstraints("Cannot satisfy " + toString(dependency.requirement()) + " on " + toString(dependency._projectIdentifier) + " with " + toString(instantiation));
        }
      }

      it->addRequirement(dependency.requirement().cloneRequirement(), std::move(dependent));
    }

    operator ArbiterResolvedDependencyGraph () const
    {
      assert(solved());

      ArbiterResolvedDependencyGraph graph;
      for (size_t i = 0; i < _variables.size(); ++i) {
        const Variable &variable = _variables.at(i);
        const Instantiation &value = *_values.at(i);

        const auto requirement = variable.requirement();
        Optional<ArbiterSelectedVersion> selectedVersion = value.bestVersionSatisfying(requirement);

        // We must find a version if the node's instantiation satisfied its
        // requirement.
        assert(selectedVersion);

        graph.addNode(ArbiterResolvedDependency(variable.projectIdentifier(), std::move(*selectedVersion)));
        for (const ArbiterDependency &dependency : value.dependencies()) {
          graph.addEdge(variable.projectIdentifier(), dependency._projectIdentifier);
        }
      }

      return graph;
    }

  private:
    std::vector<Variable> _variables;
    std::vector<std::shared_ptr<Arbiter::Instantiation>> _values;

    void removeRequirementsFrom (const ArbiterProjectIdentifier &culprit)
    {
      size_t valueCount = _values.size();

      auto it = _variables.begin();
      for (; it != _variables.begin() + valueCount; ++it) {
        Variable &variable = *it;
        assert(variable.projectIdentifier() != culprit);

        // This variable should still have other requirements applied to it,
        // because the culprit variable should have been added _after_ this
        // variable was already present.
        size_t remaining __attribute__((unused)) = variable.removeRequirementsFrom(culprit);
        assert(remaining > 0);
      }

      assert(it == _variables.begin() + valueCount);
      while (it != _variables.end()) {
        Variable &variable = *it;
        if (variable.projectIdentifier() == culprit || variable.removeRequirementsFrom(culprit) > 0) {
          ++it;
        } else {
          it = _variables.erase(it);
        }
      }
    }
};

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
  startStats();

  Network network;
  for (const ArbiterDependency &dependency : _dependenciesToResolve._dependencies) {
    network.enqueueDependency(dependency, None());
  }

  while (!network.solved()) {
    try {
      const Variable &variable = network.nextUnsolvedVariable();
      const ArbiterProjectIdentifier &projectIdentifier = variable.projectIdentifier();
      const auto requirement = variable.requirement();

      std::shared_ptr<Instantiation> instantiationPtr = bestProjectInstantiationSatisfying(projectIdentifier, requirement);
      if (!instantiationPtr) {
        throw Exception::UnsatisfiableConstraints("Cannot satisfy " + toString(requirement) + " from available versions of " + toString(projectIdentifier));
      }

      const Instantiation &instantiation = *instantiationPtr;
      network.solveNextVariable(std::move(instantiationPtr));

      for (const ArbiterDependency &dependency : instantiation.dependencies()) {
        network.enqueueDependency(dependency, makeOptional(projectIdentifier));
      }
    } catch (Arbiter::Exception::UserError &) {
      // Always consider errors in user-provided callbacks to be fatal.
      throw;
    } catch (Arbiter::Exception::Base &) {
      ++_latestStats._deadEnds;

      if (!network.backtrack()) {
        throw;
      }
    }
  }

  auto graph = static_cast<ArbiterResolvedDependencyGraph>(std::move(network));
  endStats();

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
