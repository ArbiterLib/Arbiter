#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <arbiter/Requirement.h>

#include "Version.h"

#include <memory>
#include <ostream>

struct ArbiterRequirement
{
  public:
    virtual ~ArbiterRequirement () = default;

    /**
     * Returns whether this requirement would be satisfied by using the given
     * version.
     */
    virtual bool satisfiedBy (const ArbiterSemanticVersion &version) const noexcept = 0;

    /**
     * Returns whether this requirement is equivalent to the given one.
     */
    virtual bool operator== (const ArbiterRequirement &other) const noexcept = 0;

    bool operator!= (const ArbiterRequirement &other) const noexcept
    {
      return !(*this == other);
    }

    /**
     * Creates a copy of this requirement (including its dynamic type).
     */
    virtual std::unique_ptr<ArbiterRequirement> clone () const = 0;

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

  protected:
    friend struct std::hash<ArbiterRequirement>;
    friend std::ostream &operator<< (std::ostream &os, const ArbiterRequirement &requirement);

    virtual size_t hash () const noexcept = 0;
    virtual std::ostream &describe (std::ostream &os) const = 0;
};

std::ostream &operator<< (std::ostream &os, const ArbiterRequirement &requirement);

namespace Arbiter {
namespace Requirement {

/**
 * A requirement satisfied by any version.
 */
class Any final : public ArbiterRequirement
{
  public:
    bool satisfiedBy (const ArbiterSemanticVersion &) const noexcept override
    {
      return true;
    }

    bool operator== (const ArbiterRequirement &other) const noexcept override
    {
      return (bool)dynamic_cast<const Any *>(&other);
    }

    std::unique_ptr<ArbiterRequirement> clone () const override
    {
      return std::make_unique<Any>(*this);
    }

    std::unique_ptr<ArbiterRequirement> intersect (const ArbiterRequirement &rhs) const override;

  protected:
    size_t hash () const noexcept override
    {
      return 4;
    }

    std::ostream &describe (std::ostream &os) const override;
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

    bool satisfiedBy (const ArbiterSemanticVersion &version) const noexcept override
    {
      return version >= _minimumVersion;
    }

    bool operator== (const ArbiterRequirement &other) const noexcept override;

    std::unique_ptr<ArbiterRequirement> clone () const override
    {
      return std::make_unique<AtLeast>(*this);
    }

    std::unique_ptr<ArbiterRequirement> intersect (const ArbiterRequirement &rhs) const override;

  protected:
    size_t hash () const noexcept override;
    std::ostream &describe (std::ostream &os) const override;
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

    bool satisfiedBy (const ArbiterSemanticVersion &version) const noexcept override;
    bool operator== (const ArbiterRequirement &other) const noexcept override;

    std::unique_ptr<ArbiterRequirement> clone () const override
    {
      return std::make_unique<CompatibleWith>(*this);
    }

    std::unique_ptr<ArbiterRequirement> intersect (const ArbiterRequirement &rhs) const override;

  protected:
    size_t hash () const noexcept override;
    std::ostream &describe (std::ostream &os) const override;
};

/**
 * A requirement satisfied only by one exact version.
 */
class Exactly final : public ArbiterRequirement
{
  public:
    ArbiterSemanticVersion _version;

    explicit Exactly (ArbiterSemanticVersion version) noexcept
      : _version(std::move(version))
    {}

    bool satisfiedBy (const ArbiterSemanticVersion &version) const noexcept override
    {
      return version == _version;
    }

    bool operator== (const ArbiterRequirement &other) const noexcept override;

    std::unique_ptr<ArbiterRequirement> clone () const override
    {
      return std::make_unique<Exactly>(*this);
    }

    std::unique_ptr<ArbiterRequirement> intersect (const ArbiterRequirement &rhs) const override;

  protected:
    size_t hash () const noexcept override;
    std::ostream &describe (std::ostream &os) const override;
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
