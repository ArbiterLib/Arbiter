#ifndef ARBITER_REQUIREMENT_INL_H
#define ARBITER_REQUIREMENT_INL_H

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include "Requirement.h"
#include "Version-inl.h"

struct ArbiterRequirement
{
  public:
    virtual ~ArbiterRequirement () = default;

    virtual bool satisfiedBy (const ArbiterSemanticVersion &version) const noexcept = 0;

    virtual bool operator== (const ArbiterRequirement &other) const noexcept = 0;

    bool operator!= (const ArbiterRequirement &other) const noexcept
    {
      return !(*this == other);
    }
};

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
};

class AtLeast : public ArbiterRequirement
{
  public:
    explicit AtLeast (ArbiterSemanticVersion version)
      : _minimumVersion(version)
    {}

    bool satisfiedBy (const ArbiterSemanticVersion &version) const noexcept override
    {
      return version >= _minimumVersion;
    }

    bool operator== (const ArbiterRequirement &other) const noexcept override;

  private:
    ArbiterSemanticVersion _minimumVersion;
};

class CompatibleWith : public ArbiterRequirement
{
  public:
    explicit CompatibleWith (ArbiterSemanticVersion version, ArbiterRequirementStrictness strictness)
      : _baseVersion(version)
      , _strictness(strictness)
    {}

    bool satisfiedBy (const ArbiterSemanticVersion &version) const noexcept override;
    bool operator== (const ArbiterRequirement &other) const noexcept override;

  private:
    ArbiterSemanticVersion _baseVersion;
    ArbiterRequirementStrictness _strictness;
};

class Exactly : public ArbiterRequirement
{
  public:
    explicit Exactly (ArbiterSemanticVersion version)
      : _version(version)
    {}

    bool satisfiedBy (const ArbiterSemanticVersion &version) const noexcept override
    {
      return version == _version;
    }

    bool operator== (const ArbiterRequirement &other) const noexcept override;

  private:
    ArbiterSemanticVersion _version;
};

}
}

#endif
