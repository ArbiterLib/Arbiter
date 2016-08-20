#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include "Hash.h"
#include "Version.h"

#include <arbiter/Requirement.h>

#include <memory>
#include <ostream>

struct ArbiterRequirement
{
  public:
    virtual ~ArbiterRequirement () = default;

    virtual bool satisfiedBy (const ArbiterSemanticVersion &version) const noexcept = 0;

    virtual bool operator== (const ArbiterRequirement &other) const noexcept = 0;
    virtual size_t hash () const noexcept = 0;
    virtual std::unique_ptr<ArbiterRequirement> clone () const = 0;
    virtual std::ostream &describe (std::ostream &os) const = 0;
    virtual std::unique_ptr<ArbiterRequirement> intersect (const ArbiterRequirement &rhs) const = 0;

    bool operator!= (const ArbiterRequirement &other) const noexcept
    {
      return !(*this == other);
    }
};

std::ostream &operator<< (std::ostream &os, const ArbiterRequirement &requirement);

namespace Arbiter {
namespace Requirement {

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

    size_t hash () const noexcept override
    {
      return 4;
    }

    std::unique_ptr<ArbiterRequirement> clone () const override
    {
      return std::make_unique<Any>(*this);
    }

    std::ostream &describe (std::ostream &os) const override;
    std::unique_ptr<ArbiterRequirement> intersect (const ArbiterRequirement &rhs) const override;
};

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

    size_t hash () const noexcept override
    {
      return hashOf(_minimumVersion);
    }

    std::unique_ptr<ArbiterRequirement> clone () const override
    {
      return std::make_unique<AtLeast>(*this);
    }

    std::ostream &describe (std::ostream &os) const override;
    std::unique_ptr<ArbiterRequirement> intersect (const ArbiterRequirement &rhs) const override;
};

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

    size_t hash () const noexcept override
    {
      return hashOf(_baseVersion);
    }

    std::unique_ptr<ArbiterRequirement> clone () const override
    {
      return std::make_unique<CompatibleWith>(*this);
    }

    std::ostream &describe (std::ostream &os) const override;
    std::unique_ptr<ArbiterRequirement> intersect (const ArbiterRequirement &rhs) const override;
};

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

    size_t hash () const noexcept override
    {
      return hashOf(_version);
    }

    std::unique_ptr<ArbiterRequirement> clone () const override
    {
      return std::make_unique<Exactly>(*this);
    }

    std::ostream &describe (std::ostream &os) const override;
    std::unique_ptr<ArbiterRequirement> intersect (const ArbiterRequirement &rhs) const override;
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
