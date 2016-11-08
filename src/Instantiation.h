#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include "Dependency.h"
#include "Optional.h"
#include "Version.h"

#include <functional>
#include <ostream>
#include <unordered_set>

struct ArbiterRequirement;

namespace Arbiter {

/**
 * Represents an instantiation of a project in the dependency graph, across all
 * versions of that project where the dependency requirements are _identical_.
 *
 * In other words, this corresponds to a set of project versions that are
 * substitutable for each other in terms of the effect they would have upon the
 * graph. If a particular project instantiation caused the graph to become
 * inconsistent, we know that _all_ versions associated with that instantiation
 * are not viable.
 *
 * This does not take into account constraints upon the version of the project
 * itself, merely how it would affect further resolution.
 */
class Instantiation final
{
  public:
    // TODO: Enforce uniqueness upon project IDs in this set.
    using Dependencies = std::unordered_set<ArbiterDependency>;

    using Versions = std::set<ArbiterSelectedVersion, std::greater<ArbiterSelectedVersion>>;

    explicit Instantiation (Dependencies dependencies)
      : _dependencies(std::move(dependencies))
    {}

    const Dependencies &dependencies () const
    {
      return _dependencies;
    }

    /**
     * The versions which correspond to this instantiation.
     */
    Versions _versions;

    /**
     * Returns the "best" version from this instantiation which satisfies the
     * given requirement.
     */
    Optional<ArbiterSelectedVersion> bestVersionSatisfying (const ArbiterRequirement &requirement) const;

    /**
     * Determines whether any version within this instantiation can satisfy the
     * specified requirement.
     */
    bool satisfies (const ArbiterRequirement &requirement) const;

    bool operator== (const Instantiation &other) const;

  private:
    /**
     * The set of dependencies and their constraints within this instantiation.
     */
    Dependencies _dependencies;
};

std::ostream &operator<< (std::ostream &os, const Instantiation &instantiation);

} // namespace Arbiter

namespace std {

template<>
struct hash<Arbiter::Instantiation> final
{
  public:
    size_t operator() (const Arbiter::Instantiation &value) const;
};

} // namespace std
