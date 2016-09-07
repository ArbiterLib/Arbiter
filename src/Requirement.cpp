#include "Requirement.h"

#include "Hash.h"
#include "ToString.h"

#include <typeinfo>

ArbiterRequirement *ArbiterCreateRequirementAny (void)
{
  return new Arbiter::Requirement::Any;
}

ArbiterRequirement *ArbiterCreateRequirementAtLeast (const ArbiterSemanticVersion *version)
{
  return new Arbiter::Requirement::AtLeast(*version);
}

ArbiterRequirement *ArbiterCreateRequirementCompatibleWith (const ArbiterSemanticVersion *version, ArbiterRequirementStrictness strictness)
{
  return new Arbiter::Requirement::CompatibleWith(*version, strictness);
}

ArbiterRequirement *ArbiterCreateRequirementExactly (const ArbiterSemanticVersion *version)
{
  return new Arbiter::Requirement::Exactly(*version);
}

ArbiterRequirement *ArbiterCreateRequirementCompound (const ArbiterRequirement * const *requirements, size_t count)
{
  std::vector<std::shared_ptr<ArbiterRequirement>> vec;
  vec.reserve(count);

  for (size_t i = 0; i < count; i++) {
    vec.emplace_back(requirements[i]->cloneRequirement());
  }

  return new Arbiter::Requirement::Compound(std::move(vec));
}

bool ArbiterRequirementSatisfiedBy (const ArbiterRequirement *requirement, const ArbiterSemanticVersion *version)
{
  return requirement->satisfiedBy(*version);
}

std::unique_ptr<ArbiterRequirement> ArbiterRequirement::cloneRequirement () const
{
  return std::unique_ptr<ArbiterRequirement>(dynamic_cast<ArbiterRequirement *>(clone().release()));
}

namespace Arbiter {
namespace Requirement {

namespace {

ArbiterRequirementStrictness strictestStrictness (ArbiterRequirementStrictness left, ArbiterRequirementStrictness right)
{
  switch (left) {
    case ArbiterRequirementStrictnessStrict:
      return left;

    case ArbiterRequirementStrictnessAllowVersionZeroPatches:
      return right;
  }

  __builtin_unreachable();
}

template<typename Left, typename Right>
struct Intersect final
{
  // This will fail to compile if there's no specialization for Intersect<Right,
  // Left>, thereby verifying that we've handled all combinations.
  typename Intersect<Right, Left>::Result operator() (const Left &lhs, const Right &rhs) const
  {
    return Intersect<Right, Left>()(rhs, lhs);
  }
};

template<typename Other>
struct Intersect<Any, Other> final
{
  using Result = std::unique_ptr<ArbiterRequirement>;

  Result operator() (const Any &, const Other &other) const
  {
    return other.cloneRequirement();
  }
};

template<>
struct Intersect<AtLeast, AtLeast> final
{
  using Result = std::unique_ptr<ArbiterRequirement>;

  Result operator() (const AtLeast &lhs, const AtLeast &rhs) const
  {
    return std::make_unique<AtLeast>(std::max(lhs._minimumVersion, rhs._minimumVersion));
  }
};

template<>
struct Intersect<AtLeast, CompatibleWith> final
{
  using Result = std::unique_ptr<ArbiterRequirement>;

  Result operator() (const AtLeast &atLeast, const CompatibleWith &compatibleWith) const
  {
    // >= 1.2.3 vs ~> 2.0.0
    if (atLeast.satisfiedBy(compatibleWith._baseVersion)) {
      return compatibleWith.cloneRequirement();
    // ~> 1.2.3 vs >= 1.3
    } else if (compatibleWith.satisfiedBy(atLeast._minimumVersion)) {
      return std::make_unique<CompatibleWith>(atLeast._minimumVersion, compatibleWith._strictness);
    } else {
      return nullptr;
    }
  }
};

template<>
struct Intersect<CompatibleWith, CompatibleWith> final
{
  using Result = std::unique_ptr<ArbiterRequirement>;

  Result operator() (const CompatibleWith &lhs, const CompatibleWith &rhs) const
  {
    // ~> 1.2.3 vs ~> 1.4.5
    if (lhs.satisfiedBy(rhs._baseVersion)) {
      return std::make_unique<CompatibleWith>(rhs._baseVersion, strictestStrictness(lhs._strictness, rhs._strictness));
    } else if (rhs.satisfiedBy(lhs._baseVersion)) {
      return std::make_unique<CompatibleWith>(lhs._baseVersion, strictestStrictness(lhs._strictness, rhs._strictness));
    } else {
      return nullptr;
    }
  }
};

template<typename Other>
struct Intersect<Exactly, Other> final
{
  using Result = std::unique_ptr<ArbiterRequirement>;

  Result operator() (const Exactly &exactly, const Other &other) const
  {
    if (other.satisfiedBy(exactly._version)) {
      return exactly.cloneRequirement();
    } else {
      return nullptr;
    }
  }
};

template<>
struct Intersect<Exactly, Exactly> final
{
  using Result = std::unique_ptr<ArbiterRequirement>;

  Result operator() (const Exactly &lhs, const Exactly &rhs) const
  {
    if (lhs == rhs) {
      return lhs.cloneRequirement();
    } else {
      return nullptr;
    }
  }
};

template<>
struct Intersect<Compound, Compound> final
{
  using Result = std::unique_ptr<ArbiterRequirement>;

  Result operator() (const Compound &compound, const Compound &other) const
  {
    std::vector<std::shared_ptr<ArbiterRequirement>> requirements = compound._requirements;
    requirements.insert(requirements.end(), other._requirements.begin(), other._requirements.end());
    return std::make_unique<Compound>(std::move(requirements));
  }
};

template<typename Other>
struct Intersect<Compound, Other> final
{
  using Result = std::unique_ptr<ArbiterRequirement>;

  Result operator() (const Compound &compound, const Other &other) const
  {
    std::vector<std::shared_ptr<ArbiterRequirement>> requirements = compound._requirements;
    requirements.emplace_back(other.cloneRequirement());
    return std::make_unique<Compound>(std::move(requirements));
  }
};

const std::type_info &any = typeid(Any);
const std::type_info &atLeast = typeid(AtLeast);
const std::type_info &compatibleWith = typeid(CompatibleWith);
const std::type_info &exactly = typeid(Exactly);
const std::type_info &compound = typeid(Compound);

template<typename Left>
std::unique_ptr<ArbiterRequirement> intersectRight(const Left &lhs, const ArbiterRequirement &rhs)
{
  if (typeid(rhs) == any) {
    return Intersect<Left, Any>()(lhs, dynamic_cast<const Any &>(rhs));
  } else if (typeid(rhs) == atLeast) {
    return Intersect<Left, AtLeast>()(lhs, dynamic_cast<const AtLeast &>(rhs));
  } else if (typeid(rhs) == compatibleWith) {
    return Intersect<Left, CompatibleWith>()(lhs, dynamic_cast<const CompatibleWith &>(rhs));
  } else if (typeid(rhs) == exactly) {
    return Intersect<Left, Exactly>()(lhs, dynamic_cast<const Exactly &>(rhs));
  } else if (typeid(rhs) == compound) {
    return Intersect<Left, Compound>()(lhs, dynamic_cast<const Compound &>(rhs));
  } else {
    throw std::invalid_argument("Unrecognized type for requirement: " + toString(rhs));
  }
}

} // namespace

std::ostream &Any::describe (std::ostream &os) const
{
  return os << "(any version)";
}

bool AtLeast::operator== (const Base &other) const
{
  if (auto *ptr = dynamic_cast<const AtLeast *>(&other)) {
    return _minimumVersion == ptr->_minimumVersion;
  } else {
    return false;
  }
}

size_t AtLeast::hash () const noexcept
{
  return hashOf(_minimumVersion);
}

std::ostream &AtLeast::describe (std::ostream &os) const
{
  return os << ">=" << _minimumVersion;
}

bool CompatibleWith::satisfiedBy (const ArbiterSemanticVersion &version) const noexcept
{
  if (version._major != _baseVersion._major) {
    return false;
  }

  if (version._major == 0) {
    // According to SemVer, any 0.y.z release can break compatibility.
    // Therefore, minor versions need to match exactly.
    if (version._minor != _baseVersion._minor) {
      return false;
    }

    // Patch versions also technically need to match exactly, but we permit
    // choosing looser behavior.
    switch (_strictness) {
      case ArbiterRequirementStrictnessStrict:
        if (version._patch != _baseVersion._patch) {
          return false;
        }

        break;

      case ArbiterRequirementStrictnessAllowVersionZeroPatches:
        break;
    }
  }

  // Always permit prerelease strings and build metadata to vary (even on major
  // version 0), as long as the candidate version has higher precedence.
  return version >= _baseVersion;
}

bool CompatibleWith::operator== (const Base &other) const
{
  if (auto *ptr = dynamic_cast<const CompatibleWith *>(&other)) {
    return _baseVersion == ptr->_baseVersion;
  } else {
    return false;
  }
}

size_t CompatibleWith::hash () const noexcept
{
  return hashOf(_baseVersion);
}

std::ostream &CompatibleWith::describe (std::ostream &os) const
{
  return os << "~>" << _baseVersion;
}

bool Exactly::operator== (const Base &other) const
{
  if (auto *ptr = dynamic_cast<const Exactly *>(&other)) {
    return _version == ptr->_version;
  } else {
    return false;
  }
}

size_t Exactly::hash () const noexcept
{
  return hashOf(_version);
}

std::ostream &Exactly::describe (std::ostream &os) const
{
  return os << "==" << _version;
}

bool Compound::satisfiedBy (const ArbiterSemanticVersion &version) const noexcept
{
  for (const auto &requirement : _requirements) {
    if (!requirement->satisfiedBy(version)) {
      return false;
    }
  }

  return true;
}

std::ostream &Compound::describe (std::ostream &os) const
{
  os << "{ ";

  for (auto it = _requirements.begin(); it != _requirements.end(); ++it) {
    if (it == _requirements.begin()) {
      os << " && ";
    }

    os << it->get();
  }

  return os << " }";
}

bool Compound::operator== (const Arbiter::Base &other) const
{
  if (auto *ptr = dynamic_cast<const Compound *>(&other)) {
    return _requirements == ptr->_requirements;
  } else {
    return false;
  }
}

size_t Compound::hash () const noexcept
{
  size_t value = 0;

  for (const auto &requirement : _requirements) {
    value ^= hashOf(*requirement);
  }

  return value;
}

std::unique_ptr<ArbiterRequirement> Any::intersect (const ArbiterRequirement &rhs) const
{
  return intersectRight<Any>(*this, rhs);
}

std::unique_ptr<ArbiterRequirement> AtLeast::intersect (const ArbiterRequirement &rhs) const
{
  return intersectRight<AtLeast>(*this, rhs);
}

std::unique_ptr<ArbiterRequirement> CompatibleWith::intersect (const ArbiterRequirement &rhs) const
{
  return intersectRight<CompatibleWith>(*this, rhs);
}

std::unique_ptr<ArbiterRequirement> Exactly::intersect (const ArbiterRequirement &rhs) const
{
  return intersectRight<Exactly>(*this, rhs);
}

std::unique_ptr<ArbiterRequirement> Compound::intersect (const ArbiterRequirement &rhs) const
{
  return intersectRight<Compound>(*this, rhs);
}

} // namespace Requirement
} // namespace Arbiter
