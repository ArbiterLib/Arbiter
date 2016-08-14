#ifndef ARBITER_REQUIREMENT_INL_H
#define ARBITER_REQUIREMENT_INL_H

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include "Requirement.h"
#include "Version-inl.h"

class ArbiterRequirement
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
    /**
     * How strict to be in matching compatible versions.
     */
    enum Strictness
    {
      /**
       * Determine compatibility according to a strict interpretation of SemVer.
       */
      STRICT,

      /**
       * According to SemVer, technically all 0.y.z releases can break backwards
       * compatibility, meaning that minor and patch versions have to match
       * exactly in order to be "compatible."
       *
       * This looser variant permits newer patch versions, which is probably
       * closer to what the user wants.
       */
      ALLOW_VERSION_ZERO_PATCHES
    };
  
    explicit CompatibleWith (ArbiterSemanticVersion version, Strictness strictness)
      : _baseVersion(version)
      , _strictness(strictness)
    {}

    bool satisfiedBy (const ArbiterSemanticVersion &version) const noexcept override;
    bool operator== (const ArbiterRequirement &other) const noexcept override;

  private:
    ArbiterSemanticVersion _baseVersion;
    Strictness _strictness;
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
