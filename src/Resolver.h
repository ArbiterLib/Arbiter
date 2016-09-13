#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <arbiter/Resolver.h>

#include "Dependency.h"
#include "Types.h"
#include "Version.h"

#include <unordered_map>
#include <vector>

struct ArbiterResolver final : public Arbiter::Base
{
  public:
    const void *_context;

    ArbiterResolver (ArbiterResolverBehaviors behaviors, ArbiterDependencyList dependencyList, const void *context)
      : _context(context)
      , _behaviors(std::move(behaviors))
      , _dependencyList(std::move(dependencyList))
    {
      assert(_behaviors.createDependencyList);
      assert(_behaviors.createAvailableVersionsList);
    }

    ArbiterResolver (const ArbiterResolver &) = delete;
    ArbiterResolver &operator= (const ArbiterResolver &) = delete;

    /**
     * Fetches the list of dependencies for the given project and version.
     *
     * Returns the dependency list or throws an exception.
     */
    ArbiterDependencyList fetchDependencies (const ArbiterProjectIdentifier &project, const ArbiterSelectedVersion &version) noexcept(false);

    /**
     * Fetches the list of available versions for the given project.
     *
     * Returns the version list or throws an exception.
     */
    ArbiterSelectedVersionList fetchAvailableVersions (const ArbiterProjectIdentifier &project) noexcept(false);

    /**
     * Fetches a selected version for the given metadata string.
     *
     * Returns the selected version if found, or else None.
     */
    Arbiter::Optional<ArbiterSelectedVersion> fetchSelectedVersionForMetadata (const Arbiter::SharedUserValue<ArbiterSelectedVersion> &metadata);

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
    const ArbiterDependencyList _dependencyList;

    std::unordered_map<ArbiterResolvedDependency, ArbiterDependencyList> _cachedDependencies;
    std::unordered_map<ArbiterProjectIdentifier, ArbiterSelectedVersionList> _cachedAvailableVersions;
};
