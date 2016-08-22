#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <arbiter/Resolver.h>

#include "Dependency.h"
#include "Hash.h"
#include "Value.h"
#include "Version.h"

#include <mutex>
#include <unordered_map>
#include <unordered_set>

namespace Arbiter {
namespace Resolver {

/**
 * A node in, or being considered for, an acyclic dependency graph.
 */
class DependencyNode final
{
  public:
    struct Hash final
    {
      public:
        size_t operator() (const DependencyNode &node) const
        {
          return hashOf(node._project);
        }
    };

    const ArbiterProjectIdentifier _project;

    /**
     * The version of the dependency that this node represents.
     *
     * This version is merely "proposed" because it depends on the final
     * resolution of the graph, as well as whether any "better" graphs exist.
     */
    const ArbiterSelectedVersion _proposedVersion;

    DependencyNode (ArbiterProjectIdentifier project, ArbiterSelectedVersion proposedVersion, const ArbiterRequirement &requirement)
      : _project(std::move(project))
      , _proposedVersion(std::move(proposedVersion))
      , _state(std::make_shared<State>())
    {
      setRequirement(requirement);
    }

    DependencyNode (const DependencyNode &other) noexcept
      : _project(other._project)
      , _proposedVersion(other._proposedVersion)
      , _state(other._state)
    {}

    DependencyNode (DependencyNode &&other) noexcept
      : _project(other._project)
      , _proposedVersion(other._proposedVersion)
      , _state(std::move(other._state))
    {}

    DependencyNode &operator= (const DependencyNode &) = delete;
    DependencyNode &operator= (DependencyNode &&) = delete;

    bool operator== (const DependencyNode &other) const
    {
      // For the purposes of node lookup in a graph, this is the only field
      // which matters.
      return _project == other._project;
    }

    /**
     * The current requirement(s) applied to this dependency.
     *
     * This specifier may change as the graph is added to, and the requirements
     * become more stringent.
     */
    const ArbiterRequirement &requirement () const noexcept
    {
      return *_state->_requirement;
    }

    void setRequirement (const ArbiterRequirement &requirement);
    void setRequirement (std::unique_ptr<ArbiterRequirement> requirement);

    std::unordered_set<DependencyNode, Hash> &dependencies () noexcept
    {
      return _state->_dependencies;
    }

    const std::unordered_set<DependencyNode, Hash> &dependencies () const noexcept
    {
      return _state->_dependencies;
    }

  private:
    struct State final
    {
      public:
        std::unique_ptr<ArbiterRequirement> _requirement;
        std::unordered_set<DependencyNode, Hash> _dependencies;
    };

    std::shared_ptr<State> _state;
};

} // namespace Resolver
} // namespace Arbiter

namespace std {

template<>
struct hash<Arbiter::Resolver::DependencyNode> final
{
  public:
    size_t operator() (const Arbiter::Resolver::DependencyNode &node) const
    {
      return Arbiter::Resolver::DependencyNode::Hash()(node);
    }
};

} // namespace std

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
     * A full list of all nodes included in the graph.
     */
    std::unordered_set<DependencyNode> _allNodes;

    /**
     * Maps between nodes in the graph and their immediate dependencies.
     */
    std::unordered_map<DependencyNode, std::unordered_set<DependencyNode>> _edges;

    /**
     * The root nodes of the graph (i.e., those dependencies that are listed by
     * the top-level project).
     */
    std::unordered_set<DependencyNode> _roots;

    bool operator== (const DependencyGraph &other) const;

    /**
     * Attempts to add the given node into the graph, as a dependency of
     * `dependent` if specified.
     *
     * If the given node refers to a project which already exists in the graph,
     * this method will attempt to intersect the version requirements of both.
     *
     * Returns a pointer to the node as actually inserted into the graph (which
     * may be different from the node passed in), or `nullptr` if this addition
     * would make the graph inconsistent.
     */
    DependencyNode *addNode (const DependencyNode &inputNode, const DependencyNode *dependent);
};

} // namespace Resolver
} // namespace Arbiter

std::ostream &operator<< (std::ostream &os, const Arbiter::Resolver::DependencyNode &node);
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
