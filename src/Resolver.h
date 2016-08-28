#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <arbiter/Resolver.h>

#include "Dependency.h"
#include "Value.h"
#include "Version.h"

#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Arbiter {
namespace Resolver {

/**
 * Represents an acyclic dependency graph in which each project appears at most
 * once.
 *
 * Dependency graphs can exist in an incomplete state, but will never be
 * inconsistent (i.e., include versions that are known to be invalid given the
 * current graph).
 */
class DependencyGraph final
{
  public:
    /**
     * Attempts to add the given node into the graph, as a dependency of
     * `dependent` if specified.
     *
     * If the given node refers to a project which already exists in the graph,
     * this method will attempt to intersect the version requirements of both.
     *
     * Throws an exception if this addition would make the graph inconsistent.
     */
    void addNode (ArbiterResolvedDependency node, const ArbiterRequirement &initialRequirement, const Optional<ArbiterProjectIdentifier> &dependent) noexcept(false);

    /**
     * Returns a list of all nodes in the graph, in no particular order. There
     * are guaranteed to be no duplicate projects in the vector.
     */
    std::vector<ArbiterResolvedDependency> allNodes () const;

    std::ostream &describe (std::ostream &os) const;

  private:
    using NodeKey = ArbiterProjectIdentifier;

    struct NodeValue final
    {
      public:
        const ArbiterSelectedVersion _version;

        NodeValue (ArbiterSelectedVersion version, const ArbiterRequirement &requirement);

        const ArbiterRequirement &requirement () const;

        void setRequirement (const ArbiterRequirement &requirement);
        void setRequirement (std::unique_ptr<ArbiterRequirement> requirement);

      private:
        std::shared_ptr<ArbiterRequirement> _requirement;
    };

    static ArbiterResolvedDependency resolveNode (const NodeKey &key, const NodeValue &value);
    ArbiterResolvedDependency resolveNode (const NodeKey &key) const;

    std::set<NodeKey> _roots;
    std::map<NodeKey, std::unordered_set<NodeKey>> _edges;
    std::unordered_map<NodeKey, NodeValue> _nodeMap;
};

} // namespace Resolver
} // namespace Arbiter

std::ostream &operator<< (std::ostream &os, const Arbiter::Resolver::DependencyGraph &graph);

struct ArbiterResolver final
{
  public:
    using Context = Arbiter::SharedUserValue<ArbiterResolver>;

    Context _context;

    ArbiterResolver (ArbiterResolverBehaviors behaviors, ArbiterDependencyList dependencyList, Context context)
      : _context(std::move(context))
      , _behaviors(std::move(behaviors))
      , _dependencyList(std::move(dependencyList))
    {
      assert(_behaviors.createDependencyList);
      assert(_behaviors.createAvailableVersionsList);
    }

    /**
     * Fetches the list of dependencies for the given project and version.
     *
     * Returns the dependency list or throws an exception.
     */
    ArbiterDependencyList fetchDependencies (const ArbiterProjectIdentifier &project, const ArbiterSelectedVersion &version) const noexcept(false);

    /**
     * Fetches the list of available versions for the given project.
     *
     * Returns the version list or throws an exception.
     */
    ArbiterSelectedVersionList fetchAvailableVersions (const ArbiterProjectIdentifier &project) const noexcept(false);

    /**
     * Computes a list of available versions for the specified project which
     * satisfy the given requirement.
     */
    std::vector<ArbiterSelectedVersion> availableVersionsSatisfying (const ArbiterProjectIdentifier &project, const ArbiterRequirement &requirement) const noexcept(false);

    /**
     * Attempts to resolve all dependencies.
     */
    ArbiterResolvedDependencyList resolve () noexcept(false);

  private:
    const ArbiterResolverBehaviors _behaviors;
    const ArbiterDependencyList _dependencyList;

    Arbiter::Resolver::DependencyGraph resolveDependencies (const Arbiter::Resolver::DependencyGraph &baseGraph, const std::set<ArbiterDependency> &dependencySet, const std::unordered_map<ArbiterProjectIdentifier, ArbiterProjectIdentifier> &dependentsByProject) noexcept(false);
};
