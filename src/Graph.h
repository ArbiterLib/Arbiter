#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <arbiter/Graph.h>

#include "Dependency.h"
#include "Optional.h"
#include "Types.h"

#include <memory>
#include <ostream>
#include <unordered_map>
#include <vector>

struct ArbiterResolvedDependencyGraph final : public Arbiter::Base
{
  public:
    struct NodeValue final
    {
      public:
        const ArbiterSelectedVersion _version;

        NodeValue (ArbiterSelectedVersion version, const ArbiterRequirement &requirement);

        const ArbiterRequirement &requirement () const
        {
          return *_requirement;
        }

        bool operator== (const NodeValue &other) const;

      private:
        friend struct ArbiterResolvedDependencyGraph;

        std::shared_ptr<ArbiterRequirement> _requirement;

        void setRequirement (const ArbiterRequirement &requirement);
        void setRequirement (std::unique_ptr<ArbiterRequirement> requirement);
    };

    using NodeKey = ArbiterProjectIdentifier;
    using NodeMap = std::unordered_map<NodeKey, NodeValue>;
    using EdgeMap = std::unordered_map<NodeKey, std::set<NodeKey>>;

    /**
     * Attempts to add the given node into the graph, as a dependency of
     * `dependent` if specified.
     *
     * If the given node refers to a project which already exists in the graph,
     * this method will attempt to intersect the version requirements of both.
     *
     * Throws an exception if this addition would make the graph inconsistent.
     */
    void addNode (ArbiterResolvedDependency node, const ArbiterRequirement &initialRequirement, const Arbiter::Optional<ArbiterProjectIdentifier> &dependent) noexcept(false);

    const NodeMap &nodes () const
    {
      return _nodes;
    }

    const EdgeMap &edges () const
    {
      return _edges;
    }

    static ArbiterResolvedDependency resolveNode (const NodeMap::value_type &node);
    ArbiterResolvedDependency resolveNode (const NodeMap::key_type &key) const;

    ArbiterResolvedDependencyInstaller createInstaller () const;

    std::unique_ptr<Arbiter::Base> clone () const override;
    std::ostream &describe (std::ostream &os) const override;
    bool operator== (const Arbiter::Base &other) const override;

  private:
    EdgeMap _edges;
    NodeMap _nodes;
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
