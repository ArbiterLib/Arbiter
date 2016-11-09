#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <arbiter/Resolver.h>

#include "Dependency.h"
#include "Graph.h"
#include "Instantiation.h"
#include "Project.h"
#include "Stats.h"
#include "Types.h"
#include "Version.h"

#include <memory>
#include <unordered_map>
#include <vector>

struct ArbiterResolver final : public Arbiter::Base
{
  public:
    std::shared_ptr<const void> _context;

    // Statistics from the latest dependency resolution.
    Arbiter::Stats _latestStats;

    ArbiterResolver (ArbiterResolverBehaviors behaviors, ArbiterResolvedDependencyGraph initialGraph, ArbiterDependencyList dependenciesToResolve, std::shared_ptr<const void> context)
      : _context(std::move(context))
      , _behaviors(std::move(behaviors))
      , _initialGraph(std::move(initialGraph))
      , _dependenciesToResolve(std::move(dependenciesToResolve))
    {
      assert(_behaviors.createDependencyList);
      assert(_behaviors.createAvailableVersionsList);
    }

    ArbiterResolver (const ArbiterResolver &) = delete;
    ArbiterResolver &operator= (const ArbiterResolver &) = delete;

    /**
     * Fetches the dependencies for the given project and version.
     *
     * Returns the dependencies or throws an exception.
     */
    const Arbiter::Instantiation::Dependencies &fetchDependencies (const ArbiterProjectIdentifier &projectIdentifier, const ArbiterSelectedVersion &version) noexcept(false);

    /**
     * Fetches the available versions for the given project.
     *
     * Returns the versions or throws an exception.
     */
    const Arbiter::Project::Domain &fetchAvailableVersions (const ArbiterProjectIdentifier &projectIdentifier) noexcept(false);

    /**
     * Fetches a selected version for the given metadata string.
     *
     * Returns the selected version if found, or else None.
     */
    Arbiter::Optional<ArbiterSelectedVersion> fetchSelectedVersionForMetadata (const ArbiterProjectIdentifier &project, const Arbiter::SharedUserValue<ArbiterSelectedVersion> &metadata);

    /**
     * Computes a list of available versions for the specified project which
     * satisfy the given requirement.
     */
    std::vector<ArbiterSelectedVersion> availableVersionsSatisfying (const ArbiterProjectIdentifier &project, const ArbiterRequirement &requirement) noexcept(false);

    /**
     * Attempts to resolve all dependencies.
     */
    ArbiterResolvedDependencyGraph resolve () noexcept(false);

    std::unique_ptr<Arbiter::Base> clone () const override;
    std::ostream &describe (std::ostream &os) const override;
    bool operator== (const Arbiter::Base &other) const override;

  private:
    const ArbiterResolverBehaviors _behaviors;
    const ArbiterResolvedDependencyGraph _initialGraph;
    const ArbiterDependencyList _dependenciesToResolve;

    std::unordered_map<ArbiterProjectIdentifier, Arbiter::Project> _projects;

    void startStats ();
    void endStats ();
};
