#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <arbiter/Requirement.h>

#include "Types.h"
#include "Version.h"

#include <memory>
#include <ostream>

struct ArbiterRequirement : public Arbiter::Base
{
  public:
    /**
     * Returns whether this requirement would be satisfied by using the given
     * selected version.
     */
    virtual ArbiterRequirementSuitability satisfiedBy (const ArbiterSelectedVersion &selectedVersion) const = 0;

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

    std::unique_ptr<ArbiterRequirement> cloneRequirement () const;
    virtual size_t hash () const noexcept = 0;
};

namespace Arbiter {
namespace Requirement {

/**
 * A requirement satisfied by any version.
 */
class Any final : public ArbiterRequirement
{
  public:
    ArbiterRequirementSuitability satisfiedBy (const ArbiterSelectedVersion &) const override
    {
      return ArbiterRequirementSuitabilitySuitable;
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

    bool satisfiedBy (const ArbiterSemanticVersion &version) const noexcept
    {
      return version >= _minimumVersion;
    }

    ArbiterRequirementSuitability satisfiedBy (const ArbiterSelectedVersion &selectedVersion) const override;

    std::unique_ptr<Base> clone () const override
    {
      return std::make_unique<AtLeast>(*this);
    }

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

    std::unique_ptr<Base> clone () const override
    {
      return std::make_unique<CompatibleWith>(*this);
    }

    bool satisfiedBy (const ArbiterSemanticVersion &version) const noexcept;
    ArbiterRequirementSuitability satisfiedBy (const ArbiterSelectedVersion &selectedVersion) const override;
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

    bool satisfiedBy (const ArbiterSemanticVersion &version) const noexcept
    {
      return version == _version;
    }

    ArbiterRequirementSuitability satisfiedBy (const ArbiterSelectedVersion &selectedVersion) const override;

    std::unique_ptr<Base> clone () const override
    {
      return std::make_unique<Exactly>(*this);
    }

    std::ostream &describe (std::ostream &os) const override;
    bool operator== (const Arbiter::Base &other) const override;
    std::unique_ptr<ArbiterRequirement> intersect (const ArbiterRequirement &rhs) const override;
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
