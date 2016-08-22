#include "Resolver.h"

#include "Exception.h"
#include "Hash.h"
#include "Optional.h"
#include "Requirement.h"
#include "ToString.h"

#include <cassert>
#include <exception>
#include <stdexcept>

using namespace Arbiter;
using namespace Resolver;

void DependencyNode::setRequirement (const ArbiterRequirement &requirement)
{
  setRequirement(requirement.clone());
}

void DependencyNode::setRequirement (std::unique_ptr<ArbiterRequirement> requirement)
{
  assert(requirement);
  assert(requirement->satisfiedBy(_proposedVersion._semanticVersion));

  _state->_requirement = std::move(requirement); 
}

std::ostream &operator<< (std::ostream &os, const DependencyNode &node)
{
  return os
    << node._project << " @ " << node._proposedVersion
    << " (restricted to " << node.requirement() << ")";
}

bool DependencyGraph::operator== (const DependencyGraph &other) const
{
  return _edges == other._edges && _roots == other._roots;
}

DependencyNode *DependencyGraph::addNode (const DependencyNode &inputNode, const DependencyNode *dependent)
{
  auto nodeInsertion = _allNodes.insert(inputNode);

  // Unordered collections rightly discourage mutation so hashes don't get
  // invalidated, but we've already handled this in the implementation of
  // DependencyNode.
  DependencyNode &insertedNode = const_cast<DependencyNode &>(*nodeInsertion.first);

  // If no insertion was actually performed, we need to unify our input with
  // what was already there.
  if (!nodeInsertion.second) {
    if (auto newRequirement = insertedNode.requirement().intersect(inputNode.requirement())) {
      if (!newRequirement->satisfiedBy(insertedNode._proposedVersion._semanticVersion)) {
        // This strengthened requirement invalidates the version we've
        // proposed in this graph, so the graph would become inconsistent.
        return nullptr;
      }

      insertedNode.setRequirement(std::move(newRequirement));
    } else {
      // If intersecting the requirements is impossible, the versions
      // currently shouldn't be able to match.
      //
      // Notably, though, Carthage does permit scenarios like this when
      // pinned to a branch. Arbiter doesn't support requirements like this
      // right now, but may in the future.
      assert(inputNode._proposedVersion != insertedNode._proposedVersion);
      return nullptr;
    }
  }

  if (dependent) {
    _edges[*dependent].insert(insertedNode);
  } else {
    _roots.insert(insertedNode);
  }

  return &insertedNode;
}

std::ostream &operator<< (std::ostream &os, const DependencyGraph &graph)
{
  os << "Roots:";
  for (const DependencyNode &root : graph._roots) {
    os << "\n\t" << root;
  }

  os << "\n\nEdges";
  for (const auto &pair : graph._edges) {
    const DependencyNode &node = pair.first;
    os << "\n\t" << node._project << " ->";

    const auto &dependencies = pair.second;
    for (const DependencyNode &dep : dependencies) {
      os << "\n\t\t" << dep;
    }
  }

  return os;
}

ArbiterResolver *ArbiterCreateResolver (ArbiterResolverBehaviors behaviors, const ArbiterDependencyList *dependencyList, ArbiterUserValue context)
{
  return new ArbiterResolver(std::move(behaviors), *dependencyList, ArbiterResolver::Context(std::move(context)));
}

const void *ArbiterResolverContext (const ArbiterResolver *resolver)
{
  return resolver->_context.data();
}

bool ArbiterResolvedAllDependencies (const ArbiterResolver *resolver)
{
  return resolver->resolvedAll();
}

ArbiterResolvedDependency *ArbiterCreateNextResolvedDependency (ArbiterResolver *resolver, char **error)
{
  Optional<ArbiterResolvedDependency> dependency;

  try {
    dependency = resolver->resolveNext();
  } catch (const std::exception &ex) {
    if (error) {
      *error = copyCString(ex.what()).release();
    }

    return nullptr;
  }

  return new ArbiterResolvedDependency(std::move(*dependency));
}

void ArbiterFreeResolver (ArbiterResolver *resolver)
{
  delete resolver;
}

ArbiterDependencyList ArbiterResolver::fetchDependencies (const ArbiterProjectIdentifier &project, const ArbiterSelectedVersion &version) const noexcept(false)
{
  char *error = nullptr;
  std::unique_ptr<ArbiterDependencyList> dependencyList(_behaviors.createDependencyList(this, &project, &version, &error));

  if (dependencyList) {
    assert(!error);
    return *dependencyList;
  } else if (error) {
    throw UserError(copyAcquireCString(error));
  } else {
    throw UserError();
  }
}

ArbiterSelectedVersionList ArbiterResolver::fetchAvailableVersions (const ArbiterProjectIdentifier &project) const noexcept(false)
{
  char *error = nullptr;
  std::unique_ptr<ArbiterSelectedVersionList> versionList(_behaviors.createAvailableVersionsList(this, &project, &error));

  if (versionList) {
    assert(!error);
    return *versionList;
  } else if (error) {
    throw UserError(copyAcquireCString(error));
  } else {
    throw UserError();
  }
}

bool ArbiterResolver::resolvedAll () const noexcept
{
  return _remainingDependencies._dependencies.empty();
}

ArbiterResolvedDependency resolveNext() noexcept(false)
{
  assert(false);
}
