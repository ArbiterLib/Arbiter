#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <arbiter/Dependency.h>

#include "Optional.h"
#include "Project.h"
#include "Requirement.h"
#include "Types.h"
#include "Value.h"
#include "Version.h"

#include <functional>
#include <memory>
#include <ostream>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <vector>

struct ArbiterDependency final : public Arbiter::Base
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

    std::unique_ptr<Arbiter::Base> clone () const override;
    std::ostream &describe (std::ostream &os) const override;
    bool operator== (const Arbiter::Base &other) const override;

    bool operator< (const ArbiterDependency &other) const
    {
      return _projectIdentifier < other._projectIdentifier;
    }

  private:
    std::unique_ptr<ArbiterRequirement> _requirement;
};

struct ArbiterDependencyList final : public Arbiter::Base
{
  public:
    std::vector<ArbiterDependency> _dependencies;

    ArbiterDependencyList () = default;

    explicit ArbiterDependencyList (std::vector<ArbiterDependency> dependencies)
      : _dependencies(std::move(dependencies))
    {}

    ArbiterDependencyList (const ArbiterDependencyList &) = default;
    ArbiterDependencyList &operator= (const ArbiterDependencyList &) = default;

    ArbiterDependencyList (ArbiterDependencyList &&) = default;
    ArbiterDependencyList &operator= (ArbiterDependencyList &&) = default;

    std::unique_ptr<Arbiter::Base> clone () const override;
    std::ostream &describe (std::ostream &os) const override;
    bool operator== (const Arbiter::Base &other) const override;
};

struct ArbiterResolvedDependency final : public Arbiter::Base
{
  public:
    ArbiterProjectIdentifier _project;
    ArbiterSelectedVersion _version;

    ArbiterResolvedDependency (ArbiterProjectIdentifier project, ArbiterSelectedVersion version)
      : _project(std::move(project))
      , _version(std::move(version))
    {}

    std::unique_ptr<Arbiter::Base> clone () const override;
    std::ostream &describe (std::ostream &os) const override;
    bool operator== (const Arbiter::Base &other) const override;

    bool operator< (const ArbiterResolvedDependency &other) const;
};

namespace std {

template<>
struct hash<ArbiterProjectIdentifier> final
{
  public:
    size_t operator() (const ArbiterProjectIdentifier &project) const;
};

template<>
struct hash<ArbiterDependency> final
{
  public:
    size_t operator() (const ArbiterDependency &dependency) const;
};

template<>
struct hash<ArbiterResolvedDependency> final
{
  public:
    size_t operator() (const ArbiterResolvedDependency &dependency) const;
};

} // namespace std
