#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <arbiter/Graph.h>

#include "Dependency.h"
#include "Optional.h"
#include "Types.h"

#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <vector>

struct ArbiterResolvedDependencyGraph final : public Arbiter::Base
{
  public:
    struct Node final
    {
      public:
        Node (ArbiterSelectedVersion version)
          : _version(std::move(version))
        {}

        std::set<ArbiterProjectIdentifier> _dependencies;

        const ArbiterSelectedVersion &version () const
        {
          return _version;
        }

        bool operator== (const Node &other) const;

      private:
        ArbiterSelectedVersion _version;
    };

    using NodeMap = std::map<ArbiterProjectIdentifier, Node>;

    /**
     * Attempts to add the given node into the graph.
     *
     * Throws an exception if the node refers to a project which already exists
     * in the graph, but with a different version.
     */
    void addNode (const ArbiterResolvedDependency &node) noexcept(false);

    /**
     * Adds an edge from a dependent to its dependency.
     *
     * Both sides of the edge must have already been added to the graph with
     * addNode().
     */
    void addEdge (const ArbiterProjectIdentifier &dependent, ArbiterProjectIdentifier dependency);

    const NodeMap &nodes () const
    {
      return _nodes;
    }

    ArbiterResolvedDependency resolveNode (const ArbiterProjectIdentifier &projectIdentifier) const;

    ArbiterResolvedDependencyInstaller createInstaller () const;

    /**
     * Creates a new dependency graph that contains only nodes and edges which
     * are reachable from the nodes referenced by `roots`.
     *
     * It is an error to include a node here which doesn't appear in the graph.
     */
    ArbiterResolvedDependencyGraph graphWithNewRoots (const std::vector<ArbiterProjectIdentifier> &roots) const;

    std::unique_ptr<Arbiter::Base> clone () const override;
    std::ostream &describe (std::ostream &os) const override;
    bool operator== (const Arbiter::Base &other) const override;

  private:
    NodeMap _nodes;

    void walkNodeAndCopyInto (ArbiterResolvedDependencyGraph &newGraph, const ArbiterProjectIdentifier &key, const Arbiter::Optional<ArbiterProjectIdentifier> &dependent) const;
};

struct ArbiterResolvedDependencyInstaller final : public Arbiter::Base
{
  public:
    using SortedEdgesMap = std::unordered_map<ArbiterProjectIdentifier, std::vector<ArbiterProjectIdentifier>>;
    using PhaseSet = std::set<ArbiterResolvedDependency>;

    std::vector<PhaseSet> _phases;
    SortedEdgesMap _edges;

    ArbiterResolvedDependencyInstaller () = default;

    size_t countInPhase (size_t phaseIndex) const;

    bool contains (const ArbiterResolvedDependency &node) const;

    std::unique_ptr<Arbiter::Base> clone () const override;
    std::ostream &describe (std::ostream &os) const override;
    bool operator== (const Arbiter::Base &other) const override;
};
