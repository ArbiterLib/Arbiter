#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <arbiter/Dependency.h>

#include "Value.h"
#include "Version.h"

#include <functional>
#include <memory>
#include <ostream>
#include <vector>

struct ArbiterRequirement;

struct ArbiterProjectIdentifier final
{
  public:
    using Value = Arbiter::SharedUserValue<ArbiterProjectIdentifier>;

    Value _value;

    explicit ArbiterProjectIdentifier (Value value)
      : _value(std::move(value))
    {}

    bool operator== (const ArbiterProjectIdentifier &other) const
    {
      return _value == other._value;
    }

    bool operator!= (const ArbiterProjectIdentifier &other) const
    {
      return !(*this == other);
    }
};

std::ostream &operator<< (std::ostream &os, const ArbiterProjectIdentifier &identifier);

struct ArbiterDependency final
{
  public:
    ArbiterProjectIdentifier _projectIdentifier;

    ArbiterDependency (ArbiterProjectIdentifier projectIdentifier, const ArbiterRequirement &requirement);

    ArbiterDependency (const ArbiterDependency &other)
      : ArbiterDependency(other._projectIdentifier, other.requirement())
    {}

    ArbiterDependency &operator= (const ArbiterDependency &other);

    const ArbiterRequirement &requirement() const noexcept
    {
      return *_requirement;
    }

    bool operator== (const ArbiterDependency &other) const;

  private:
    std::unique_ptr<ArbiterRequirement> _requirement;
};

std::ostream &operator<< (std::ostream &os, const ArbiterDependency &dependency);

struct ArbiterDependencyList final
{
  public:
    std::vector<ArbiterDependency> _dependencies;

    explicit ArbiterDependencyList (std::vector<ArbiterDependency> dependencies)
      : _dependencies(std::move(dependencies))
    {}
};

std::ostream &operator<< (std::ostream &os, const ArbiterDependencyList &dependencyList);

struct ArbiterResolvedDependency final
{
  public:
    ArbiterProjectIdentifier _project;
    ArbiterSelectedVersion _version;

    ArbiterResolvedDependency (ArbiterProjectIdentifier project, ArbiterSelectedVersion version)
      : _project(std::move(project))
      , _version(std::move(version))
    {}
};

std::ostream &operator<< (std::ostream &os, const ArbiterResolvedDependency &dependency);

namespace std {

template<>
struct hash<ArbiterProjectIdentifier> final
{
  public:
    size_t operator() (const ArbiterProjectIdentifier &) const
    {
      // TODO: Need a real hash!
      return 4;
    }
};

template<>
struct hash<ArbiterDependency> final
{
  public:
    size_t operator() (const ArbiterDependency &dependency) const;
};

} // namespace std
