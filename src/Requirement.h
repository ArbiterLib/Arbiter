#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <arbiter/Requirement.h>

#include "Types.h"
#include "Version.h"

#include <cassert>
#include <memory>
#include <ostream>

namespace Arbiter {
namespace Requirement {

class Visitor;

} // namespace Requirement
} // namespace Arbiter

namespace Arbiter {

class Instantiation;

} // namespace Arbiter

struct ArbiterRequirement : public Arbiter::Base
{
  public:
    /**
     * Returns whether this requirement would be satisfied by using the given
     * selected version.
     */
    virtual bool satisfiedBy (const ArbiterSelectedVersion &selectedVersion) const = 0;

    /**
     * Returns the priority of this requirement.
     */
    virtual int priority () const noexcept
    {
      return 0;
    }

    /**
     * Attempts to create a requirement which expresses the intersection of this
     * requirement and the given one
     *
     * In other words, this attempts to find the loosest possible requirement
     * which is a superset of the two inputs. Any version which passes the
     * intersected requirement would also pass either one of the original
     * inputs.
     *
     * Returns `nullptr` if no intersection is possible.
     */
    virtual std::unique_ptr<ArbiterRequirement> intersect (const ArbiterRequirement &rhs) const = 0;

    /**
     * Visits the requirement, then any child requirements.
     *
     * The default implementation simply visits this requirement.
     */
    virtual void visit (Arbiter::Requirement::Visitor &visitor) const;

    std::unique_ptr<ArbiterRequirement> cloneRequirement () const;
    virtual size_t hash () const noexcept = 0;
};

namespace Arbiter {
namespace Requirement {

/**
 * Base class for objects that want to visit requirements.
 */
class Visitor
{
  public:
    virtual ~Visitor () = default;

    virtual void operator() (const ArbiterRequirement &requirement) = 0;
};

/**
 * A requirement satisfied by any version.
 */
class Any final : public ArbiterRequirement
{
  public:
    bool satisfiedBy (const ArbiterSemanticVersion &) const noexcept
    {
      return true;
    }

    bool satisfiedBy (const ArbiterSelectedVersion &) const override
    {
      return true;
    }

    bool operator== (const Arbiter::Base &other) const override
    {
      return (bool)dynamic_cast<const Any *>(&other);
    }

    std::unique_ptr<Arbiter::Base> clone () const override
    {
      return std::make_unique<Any>(*this);
    }

    std::ostream &describe (std::ostream &os) const override;
    std::unique_ptr<ArbiterRequirement> intersect (const ArbiterRequirement &rhs) const override;

    size_t hash () const noexcept override
    {
      return 4;
    }
};

/**
 * A requirement satisfied only by versions greater than or equal to the
 * specified one.
 */
class AtLeast final : public ArbiterRequirement
{
  public:
    ArbiterSemanticVersion _minimumVersion;

    explicit AtLeast (ArbiterSemanticVersion version) noexcept
      : _minimumVersion(std::move(version))
    {}

    bool satisfiedBy (const ArbiterSelectedVersion &selectedVersion) const override
    {
      if (selectedVersion._semanticVersion) {
        return satisfiedBy(*selectedVersion._semanticVersion);
      } else {
        return false;
      }
    }

    std::unique_ptr<Base> clone () const override
    {
      return std::make_unique<AtLeast>(*this);
    }

    bool satisfiedBy (const ArbiterSemanticVersion &version) const noexcept;
    std::ostream &describe (std::ostream &os) const override;
    bool operator== (const Arbiter::Base &other) const override;
    std::unique_ptr<ArbiterRequirement> intersect (const ArbiterRequirement &rhs) const override;
    size_t hash () const noexcept override;
};

/**
 * A requirement satisfied only by versions which are "compatible with" the
 * specified one, as defined by SemVer.
 */
class CompatibleWith final : public ArbiterRequirement
{
  public:
    ArbiterSemanticVersion _baseVersion;
    ArbiterRequirementStrictness _strictness;

    explicit CompatibleWith (ArbiterSemanticVersion version, ArbiterRequirementStrictness strictness) noexcept
      : _baseVersion(std::move(version))
      , _strictness(strictness)
    {}

    bool satisfiedBy (const ArbiterSelectedVersion &selectedVersion) const override
    {
      if (selectedVersion._semanticVersion) {
        return satisfiedBy(*selectedVersion._semanticVersion);
      } else {
        return false;
      }
    }

    std::unique_ptr<Base> clone () const override
    {
      return std::make_unique<CompatibleWith>(*this);
    }

    bool satisfiedBy (const ArbiterSemanticVersion &version) const noexcept;
    std::ostream &describe (std::ostream &os) const override;
    bool operator== (const Arbiter::Base &other) const override;
    std::unique_ptr<ArbiterRequirement> intersect (const ArbiterRequirement &rhs) const override;
    size_t hash () const noexcept override;
};

/**
 * A requirement satisfied only by one particular semantic version.
 */
class Exactly final : public ArbiterRequirement
{
  public:
    ArbiterSemanticVersion _version;

    explicit Exactly (ArbiterSemanticVersion version) noexcept
      : _version(std::move(version))
    {}

    bool satisfiedBy (const ArbiterSelectedVersion &selectedVersion) const override
    {
      if (selectedVersion._semanticVersion) {
        return satisfiedBy(*selectedVersion._semanticVersion);
      } else {
        return false;
      }
    }

    std::unique_ptr<Base> clone () const override
    {
      return std::make_unique<Exactly>(*this);
    }

    bool satisfiedBy (const ArbiterSemanticVersion &version) const noexcept;
    std::ostream &describe (std::ostream &os) const override;
    bool operator== (const Arbiter::Base &other) const override;
    std::unique_ptr<ArbiterRequirement> intersect (const ArbiterRequirement &rhs) const override;
    size_t hash () const noexcept override;
};

class Unversioned final : public ArbiterRequirement
{
  public:
    // This metadata is not really part of the requirement type; it is
    // associated with the selected version.
    using Metadata = Arbiter::SharedUserValue<ArbiterSelectedVersion>;

    Metadata _metadata;

    explicit Unversioned (Metadata metadata)
      : _metadata(std::move(metadata))
    {}

    std::unique_ptr<Base> clone () const override
    {
      return std::make_unique<Unversioned>(*this);
    }

    std::ostream &describe (std::ostream &os) const override;
    bool satisfiedBy (const ArbiterSelectedVersion &selectedVersion) const override;
    std::unique_ptr<ArbiterRequirement> intersect (const ArbiterRequirement &rhs) const override;
    bool operator== (const Arbiter::Base &other) const override;
    size_t hash () const noexcept override;
};

class Custom final : public ArbiterRequirement
{
  public:
    explicit Custom (ArbiterRequirementPredicate predicate, std::shared_ptr<const void> context)
      : _predicate(std::move(predicate))
      , _context(std::move(context))
    {
      assert(_predicate);
    }

    std::ostream &describe (std::ostream &os) const override
    {
      return os << "(custom predicate)";
    }

    std::unique_ptr<Base> clone () const override
    {
      return std::make_unique<Custom>(*this);
    }

    bool satisfiedBy (const ArbiterSelectedVersion &selectedVersion) const override;
    std::unique_ptr<ArbiterRequirement> intersect (const ArbiterRequirement &rhs) const override;
    bool operator== (const Arbiter::Base &other) const override;
    size_t hash () const noexcept override;

  private:
    ArbiterRequirementPredicate _predicate;
    std::shared_ptr<const void> _context;
};

class Compound final : public ArbiterRequirement
{
  public:
    std::vector<std::shared_ptr<ArbiterRequirement>> _requirements;

    explicit Compound (std::vector<std::shared_ptr<ArbiterRequirement>> requirements)
      : _requirements(std::move(requirements))
    {}

    std::unique_ptr<Base> clone () const override
    {
      return std::make_unique<Compound>(*this);
    }

    /**
     * Returns the minimum priority index of all the requirements held by this
     * compound requirement.
     */
    int priority () const noexcept override;

    bool satisfiedBy (const ArbiterSelectedVersion &selectedVersion) const override;
    std::unique_ptr<ArbiterRequirement> intersect (const ArbiterRequirement &rhs) const override;
    std::ostream &describe (std::ostream &os) const override;
    bool operator== (const Arbiter::Base &other) const override;
    size_t hash () const noexcept override;
    void visit (Visitor &visitor) const override;
};

class Prioritized final : public ArbiterRequirement
{
  public:
    std::shared_ptr<ArbiterRequirement> _requirement;

    explicit Prioritized (std::shared_ptr<ArbiterRequirement> requirement, int priority)
      : _requirement(std::move(requirement))
      , _priority(priority)
    {}

    std::unique_ptr<Base> clone () const override
    {
      return std::make_unique<Prioritized>(*this);
    }

    int priority () const noexcept override
    {
      return _priority;
    }

    bool satisfiedBy (const ArbiterSelectedVersion &selectedVersion) const override;
    std::unique_ptr<ArbiterRequirement> intersect (const ArbiterRequirement &rhs) const override;
    std::ostream &describe (std::ostream &os) const override;
    bool operator== (const Arbiter::Base &other) const override;
    size_t hash () const noexcept override;
    void visit (Visitor &visitor) const override;

  private:
    int _priority;
};

class ExcludedInstantiation final : public ArbiterRequirement
{
  public:
    std::shared_ptr<Instantiation> _excludedInstantiation;

    explicit ExcludedInstantiation (std::shared_ptr<Instantiation> excludedInstantiation)
      : _excludedInstantiation(std::move(excludedInstantiation))
    {}

    std::unique_ptr<Base> clone () const override
    {
      return std::make_unique<ExcludedInstantiation>(*this);
    }

    bool satisfiedBy (const ArbiterSelectedVersion &selectedVersion) const override;
    std::unique_ptr<ArbiterRequirement> intersect (const ArbiterRequirement &rhs) const override;
    std::ostream &describe (std::ostream &os) const override;
    bool operator== (const Arbiter::Base &other) const override;
    size_t hash () const noexcept override;
};

} // namespace Requirement
} // namespace Arbiter

namespace std {

template<>
struct hash<ArbiterRequirement> final
{
  public:
    size_t operator() (const ArbiterRequirement &requirement) const
    {
      return requirement.hash();
    }
};

} // namespace std
