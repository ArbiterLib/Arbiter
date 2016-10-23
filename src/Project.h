#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include "Version.h"

#include <functional>
#include <memory>
#include <set>
#include <unordered_set>
#include <vector>

struct ArbiterDependency;

namespace Arbiter {

class Instantiation; 

} // namespace Arbiter

namespace Arbiter {

/**
 * Contains the growing set of information about a project during dependency
 * resolution.
 */
class Project final
{
  public:
    using Domain = std::set<ArbiterSelectedVersion, std::greater<ArbiterSelectedVersion>>;
    using Instantiations = std::vector<std::shared_ptr<Instantiation>>;

    explicit Project (Domain domain)
      : _domain(std::move(domain))
    {}

    const Domain &domain () const
    {
      return _domain;
    }

    const Instantiations &instantiations () const
    {
      return _instantiations;
    }

    void addInstantiation (const std::shared_ptr<Instantiation> &instantiation);

    std::shared_ptr<Instantiation> instantiationForVersion (const ArbiterSelectedVersion &version) const;
    std::shared_ptr<Instantiation> instantiationForDependencies (const std::unordered_set<ArbiterDependency> &dependencies) const;

  private:
    /**
     * Possible versions for this project, in preferential order.
     */
    Domain _domain;

    /**
     * Instantiations that have been found so far. This set will only grow over
     * the course of resolution.
     */
    Instantiations _instantiations;
};

} // namespace Arbiter
