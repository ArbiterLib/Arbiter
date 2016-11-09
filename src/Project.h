#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <arbiter/Project.h>

#include "Types.h"
#include "Value.h"
#include "Version.h"

#include <functional>
#include <memory>
#include <ostream>
#include <set>
#include <unordered_set>
#include <vector>

struct ArbiterDependency;
struct ArbiterDependencyList;

namespace Arbiter {

class Instantiation; 

} // namespace Arbiter

struct ArbiterProjectIdentifier final : public Arbiter::Base
{
  public:
    using Value = Arbiter::SharedUserValue<ArbiterProjectIdentifier>;

    Value _value;

    ArbiterProjectIdentifier ()
    {}

    explicit ArbiterProjectIdentifier (Value value)
      : _value(std::move(value))
    {}

    std::unique_ptr<Arbiter::Base> clone () const override;
    std::ostream &describe (std::ostream &os) const override;
    bool operator== (const Arbiter::Base &other) const override;

    bool operator< (const ArbiterProjectIdentifier &other) const
    {
      return _value < other._value;
    }
};

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

    std::shared_ptr<Instantiation> addInstantiation (const ArbiterSelectedVersion &version, const ArbiterDependencyList &dependencyList);

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
