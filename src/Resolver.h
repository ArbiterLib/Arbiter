#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <arbiter/Resolver.h>

#include "Dependency.h"
#include "Value.h"
#include "Version.h"

#include <mutex>
#include <unordered_map>
#include <unordered_set>

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
    using Node = ArbiterResolvedDependency;
    using NodeSet = std::unordered_set<Node>;
    using EdgeMap = std::unordered_map<Node, NodeSet>;

    /**
     * Attempts to add the given node into the graph, as a dependency of
     * `dependent` if specified.
     *
     * If the given node refers to a project which already exists in the graph,
     * this method will attempt to intersect the version requirements of both.
     *
     * Returns true if the node was successfully inserted into the graph, or
     * false if this addition would make the graph inconsistent.
     */
    void addNode (Node node, const ArbiterRequirement &initialRequirement, const Node *dependent) noexcept(false);
    
    /**
     * Returns a list of all nodes in the graph. There are guaranteed to be no
     * duplicates in the vector.
     */
    std::vector<Node> allNodes() const;

    /**
     * The root nodes of the graph (i.e., those dependencies that are listed by
     * the top-level project).
     */
    const NodeSet &roots() const noexcept
    {
      return _roots;
    }

    /**
     * Maps between nodes in the graph and their immediate dependencies.
     */
    const EdgeMap &edges() const noexcept
    {
      return _edges;
    }

    bool operator== (const DependencyGraph &other) const;

  private:
    EdgeMap _edges;
    NodeSet _roots;

    /**
     * A map containing all nodes in the graph, associated with the current
     * requirement specification for each.
     *
     * The associated requirement may change as the graph is added to, and
     * therefore the requirements become more stringent.
     */
    std::unordered_map<Node, std::unique_ptr<ArbiterRequirement>> _requirementsByNode;
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
      , _remainingDependencies(std::move(dependencyList))
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
     * Returns whether all dependencies have been resolved.
     */
    bool resolvedAll () const noexcept;

    /**
     * Attempts to resolve the next unresolved dependency.
     *
     * It is invalid to invoke this method if resolvedAll() returns true.
     *
     * Returns the resolved dependency or throws an exception.
     */
    ArbiterResolvedDependency resolveNext() noexcept(false);

  private:
    const ArbiterResolverBehaviors _behaviors;
    ArbiterDependencyList _remainingDependencies;
};
