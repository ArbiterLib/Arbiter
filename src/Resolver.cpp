#include "Resolver.h"

#include "Algorithm.h"
#include "Exception.h"
#include "Iterator.h"
#include "Optional.h"
#include "Requirement.h"
#include "Stats.h"
#include "ToString.h"

#include <algorithm>
#include <cassert>
#include <exception>
#include <map>
#include <set>
#include <unordered_set>

using namespace Arbiter;

namespace {

struct UniqueDependencyHash final
{
  public:
    size_t operator() (const ArbiterDependency &dependency) const
    {
      return hashOf(dependency._projectIdentifier);
    }
};

struct UniqueDependencyEqualTo final
{
  public:
    bool operator() (const ArbiterDependency &lhs, const ArbiterDependency &rhs) const
    {
      return lhs._projectIdentifier == rhs._projectIdentifier;
    }
};

/**
 * Contains dependencies in a set where project identifier alone determines
 * uniqueness (i.e., any requirement is ignored).
 */
using UniqueDependencySet = std::unordered_set<ArbiterDependency, UniqueDependencyHash, UniqueDependencyEqualTo>;

ArbiterResolvedDependencyGraph resolveDependencies (ArbiterResolver &resolver, const ArbiterResolvedDependencyGraph &baseGraph, UniqueDependencySet dependencySet, const std::unordered_map<ArbiterProjectIdentifier, std::vector<ArbiterProjectIdentifier>> &dependentsByProject = {}) noexcept(false)
{
  if (dependencySet.empty()) {
    return baseGraph;
  }

  // This collection is reused when actually building the new dependency graph
  // below.
  std::unordered_map<ArbiterProjectIdentifier, std::unique_ptr<ArbiterRequirement>> requirementsByProject;
  requirementsByProject.reserve(dependencySet.size());

  for (const ArbiterDependency &dependency : dependencySet) {
    requirementsByProject[dependency._projectIdentifier] = dependency.requirement().cloneRequirement();
  }

  assert(requirementsByProject.size() == dependencySet.size());

  // Free the dependencySet, as it will no longer be used.
  reset(dependencySet);

  // This collection needs to exist for as long as the permuted iterators do below.
  //
  // It's important that this collection is ordered deterministically, since it
  // affects which permutations we try first.
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

      ArbiterResolvedDependencyGraph candidate = baseGraph;

      // Add everything to the graph first, to throw any exceptions that would
      // occur before we perform the computation- and memory-expensive stuff for
      // transitive dependencies.
      for (ArbiterResolvedDependency &dependency : choices) {
        const ArbiterRequirement &requirement = *requirementsByProject.at(dependency._project);
        candidate.addNode(dependency, requirement);

        auto dependents = maybeAt(dependentsByProject, dependency._project);
        if (dependents) {
          for (const ArbiterProjectIdentifier &dependent : *dependents) {
            candidate.addEdge(dependent, dependency._project);
          }
        }
      }

      // Collect immediate children for the next phase of dependency resolution,
      // so we can permute their versions as a group (for something
      // approximating breadth-first search).
      UniqueDependencySet collectedTransitives;
      std::unordered_map<ArbiterProjectIdentifier, std::vector<ArbiterProjectIdentifier>> dependentsByTransitive;

      for (ArbiterResolvedDependency &dependency : choices) {
        std::vector<ArbiterDependency> transitives = resolver.fetchDependencies(dependency._project, dependency._version)._dependencies;

        dependentsByTransitive.reserve(dependentsByTransitive.size() + transitives.size());
        for (const ArbiterDependency &transitive : transitives) {
          dependentsByTransitive[transitive._projectIdentifier].emplace_back(dependency._project);
        }

        collectedTransitives.insert(std::make_move_iterator(transitives.begin()), std::make_move_iterator(transitives.end()));
      }

      reset(choices);

      return resolveDependencies(resolver, candidate, std::move(collectedTransitives), std::move(dependentsByTransitive));
    } catch (Arbiter::Exception::Base &ex) {
      lastException = std::current_exception();
      ++resolver._latestStats._deadEnds;
    }
  }

  std::rethrow_exception(lastException);
}

class UnversionedRequirementVisitor final : public Requirement::Visitor
{
  public:
    std::vector<Requirement::Unversioned::Metadata> _allMetadata;

    void operator() (const ArbiterRequirement &requirement) override
    {
      if (const auto *ptr = dynamic_cast<const Requirement::Unversioned *>(&requirement)) {
        _allMetadata.emplace_back(ptr->_metadata);
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

ArbiterDependencyList ArbiterResolver::fetchDependencies (const ArbiterProjectIdentifier &project, const ArbiterSelectedVersion &version) noexcept(false)
{
  ArbiterResolvedDependency resolved(project, version);
  if (auto list = maybeAt(_cachedDependencies, resolved)) {
    return *list;
  }

  char *error = nullptr;
  std::unique_ptr<ArbiterDependencyList> dependencyList(_behaviors.createDependencyList(this, &project, &version, &error));

  ++_latestStats._dependencyListFetches;

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

  ++_latestStats._availableVersionFetches;

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
  UniqueDependencySet dependencySet(_dependenciesToResolve._dependencies.begin(), _dependenciesToResolve._dependencies.end());

  startStats();

  try {
    ArbiterResolvedDependencyGraph graph = resolveDependencies(*this, _initialGraph, std::move(dependencySet));
    endStats();
    return graph;
  } catch (...) {
    // TODO: Clean up with RAII?
    endStats();
    throw;
  }
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

std::vector<ArbiterSelectedVersion> ArbiterResolver::availableVersionsSatisfying (const ArbiterProjectIdentifier &project, const ArbiterRequirement &requirement) noexcept(false)
{
  std::vector<ArbiterSelectedVersion> versions;

  if (_behaviors.createSelectedVersionForMetadata) {
    UnversionedRequirementVisitor visitor;
    requirement.visit(visitor);

    for (const auto &metadata : visitor._allMetadata) {
      Optional<ArbiterSelectedVersion> version = fetchSelectedVersionForMetadata(project, metadata);
      if (version) {
        versions.emplace_back(std::move(*version));
      }
    }
  }

  std::vector<ArbiterSelectedVersion> fetchedVersions = fetchAvailableVersions(project)._versions;
  versions.insert(versions.end(), std::make_move_iterator(fetchedVersions.begin()), std::make_move_iterator(fetchedVersions.end()));

  auto removeStart = std::remove_if(versions.begin(), versions.end(), [&requirement](const ArbiterSelectedVersion &version) {
    return !requirement.satisfiedBy(version);
  });

  versions.erase(removeStart, versions.end());
  return versions;
}

void ArbiterResolver::startStats ()
{
  _latestStats = Stats(Stats::Clock::now());
}

void ArbiterResolver::endStats ()
{
  _latestStats._endTime = Stats::Clock::now();

  size_t depsSize = 0;
  for (const auto &pair : _cachedDependencies) {
    depsSize += sizeof(ArbiterResolvedDependency);
    depsSize += pair.second._dependencies.size() * sizeof(ArbiterDependency);
  }

  _latestStats._cachedDependenciesSizeEstimate = depsSize;

  size_t versionsSize = 0;
  for (const auto &pair : _cachedAvailableVersions) {
    versionsSize += sizeof(ArbiterProjectIdentifier);
    versionsSize += pair.second._versions.size() * sizeof(ArbiterSelectedVersion);
  }

  _latestStats._cachedAvailableVersionsSizeEstimate = versionsSize;
}
