#ifndef ARBITER_REQUIREMENT_INL_H
#define ARBITER_REQUIREMENT_INL_H

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include "Requirement.h"
#include "Version-inl.h"

#include <memory>
#include <ostream>

struct ArbiterRequirement
{
  public:
    virtual ~ArbiterRequirement () = default;

    virtual bool satisfiedBy (const ArbiterSemanticVersion &version) const noexcept = 0;

    virtual bool operator== (const ArbiterRequirement &other) const noexcept = 0;

    virtual std::unique_ptr<ArbiterRequirement> clone () const = 0;

    virtual std::ostream &describe (std::ostream &os) const = 0;

    bool operator!= (const ArbiterRequirement &other) const noexcept
    {
      return !(*this == other);
    }
};

std::ostream &operator<< (std::ostream &os, const ArbiterRequirement &requirement)
{
  return requirement.describe(os);
}

namespace Arbiter {
namespace Requirement {

class Any : public ArbiterRequirement
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

    std::ostream &describe (std::ostream &os) const override;
};

class AtLeast : public ArbiterRequirement
{
  public:
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

    std::ostream &describe (std::ostream &os) const override;

  private:
    ArbiterSemanticVersion _minimumVersion;
};

class CompatibleWith : public ArbiterRequirement
{
  public:
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

    std::ostream &describe (std::ostream &os) const override;

  private:
    ArbiterSemanticVersion _baseVersion;
    ArbiterRequirementStrictness _strictness;
};

class Exactly : public ArbiterRequirement
{
  public:
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

    std::ostream &describe (std::ostream &os) const override;

  private:
    ArbiterSemanticVersion _version;
};

}
}

#endif
