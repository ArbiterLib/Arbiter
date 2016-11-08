#include "Requirement.h"

#include "Hash.h"
#include "Instantiation.h"
#include "ToString.h"

#include <algorithm>
#include <limits>
#include <typeinfo>

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

template<>
struct Intersect<Exactly, AtLeast> final
{
  using Result = std::unique_ptr<ArbiterRequirement>;

  Result operator() (const Exactly &exactly, const AtLeast &other) const
  {
    if (other.satisfiedBy(exactly._version)) {
      return exactly.cloneRequirement();
    } else {
      return nullptr;
    }
  }
};

template<>
struct Intersect<Exactly, CompatibleWith> final
{
  using Result = std::unique_ptr<ArbiterRequirement>;

  Result operator() (const Exactly &exactly, const CompatibleWith &other) const
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

template<typename Other>
struct Intersect<Unversioned, Other> final
{
  using Result = std::unique_ptr<ArbiterRequirement>;

  Result operator() (const Unversioned &unversioned, const Other &other) const
  {
    std::vector<std::shared_ptr<ArbiterRequirement>> requirements = {
      std::shared_ptr<ArbiterRequirement>(other.cloneRequirement()),
      std::shared_ptr<ArbiterRequirement>(unversioned.cloneRequirement())
    };

    return std::make_unique<Compound>(std::move(requirements));
  }
};

template<typename Other>
struct Intersect<Custom, Other> final
{
  using Result = std::unique_ptr<ArbiterRequirement>;

  Result operator() (const Custom &custom, const Other &other) const
  {
    std::vector<std::shared_ptr<ArbiterRequirement>> requirements = {
      std::shared_ptr<ArbiterRequirement>(other.cloneRequirement()),
      std::shared_ptr<ArbiterRequirement>(custom.cloneRequirement())
    };

    return std::make_unique<Compound>(std::move(requirements));
  }
};

template<>
struct Intersect<Compound, Compound> final
{
  using Result = std::unique_ptr<ArbiterRequirement>;

  Result operator() (const Compound &compound, const Compound &other) const
  {
    std::vector<std::shared_ptr<ArbiterRequirement>> requirements = compound._requirements;

    requirements.reserve(requirements.size() + other._requirements.size());
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
    if (other.priority() < compound.priority()) {
      return other.cloneRequirement();
    }

    std::vector<std::shared_ptr<ArbiterRequirement>> requirements = compound._requirements;

    requirements.reserve(requirements.size() + 1);
    requirements.emplace_back(other.cloneRequirement());

    return std::make_unique<Compound>(std::move(requirements));
  }
};

template<typename Other>
struct Intersect<Prioritized, Other> final
{
  using Result = std::unique_ptr<ArbiterRequirement>;

  Result operator() (const Prioritized &prioritized, const Other &other) const
  {
    if (prioritized.priority() < other.priority()) {
      return prioritized.cloneRequirement();
    } else if (prioritized.priority() > other.priority()) {
      return other.cloneRequirement();
    } else {
      return prioritized._requirement->intersect(other);
    }
  }
};

template<typename Other>
struct Intersect<ExcludedInstantiation, Other> final
{
  using Result = std::unique_ptr<ArbiterRequirement>;

  Result operator() (const ExcludedInstantiation &req, const Other &other) const
  {
    std::vector<std::shared_ptr<ArbiterRequirement>> requirements = {
      std::shared_ptr<ArbiterRequirement>(req.cloneRequirement()),
      std::shared_ptr<ArbiterRequirement>(other.cloneRequirement())
    };

    return std::make_unique<Compound>(std::move(requirements));
  }
};

const std::type_info &any = typeid(Any);
const std::type_info &atLeast = typeid(AtLeast);
const std::type_info &compatibleWith = typeid(CompatibleWith);
const std::type_info &exactly = typeid(Exactly);
const std::type_info &unversioned = typeid(Unversioned);
const std::type_info &custom = typeid(Custom);
const std::type_info &compound = typeid(Compound);
const std::type_info &prioritized = typeid(Prioritized);
const std::type_info &excludedInstantiation = typeid(ExcludedInstantiation);

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
  } else if (typeid(rhs) == unversioned) {
    return Intersect<Left, Unversioned>()(lhs, dynamic_cast<const Unversioned &>(rhs));
  } else if (typeid(rhs) == custom) {
    return Intersect<Left, Custom>()(lhs, dynamic_cast<const Custom &>(rhs));
  } else if (typeid(rhs) == compound) {
    return Intersect<Left, Compound>()(lhs, dynamic_cast<const Compound &>(rhs));
  } else if (typeid(rhs) == prioritized) {
    return Intersect<Left, Prioritized>()(lhs, dynamic_cast<const Prioritized &>(rhs));
  } else if (typeid(rhs) == excludedInstantiation) {
    return Intersect<Left, ExcludedInstantiation>()(lhs, dynamic_cast<const ExcludedInstantiation &>(rhs));
  } else {
    throw std::invalid_argument("Unrecognized type for requirement: " + toString(rhs));
  }
}

} // namespace

std::ostream &Any::describe (std::ostream &os) const
{
  return os << "(any version)";
}

bool AtLeast::satisfiedBy (const ArbiterSemanticVersion &version) const noexcept
{
  return version >= _minimumVersion;
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

bool Exactly::satisfiedBy (const ArbiterSemanticVersion &version) const noexcept
{
  return version == _version;
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

std::ostream &Unversioned::describe (std::ostream &os) const
{
  return os << "unversioned (" << _metadata << ")";
}

bool Unversioned::satisfiedBy (const ArbiterSelectedVersion &selectedVersion) const
{
  return selectedVersion._metadata == _metadata;
}

bool Unversioned::operator== (const Base &other) const
{
  if (auto *ptr = dynamic_cast<const Unversioned *>(&other)) {
    return _metadata == ptr->_metadata;
  } else {
    return false;
  }
}

size_t Unversioned::hash () const noexcept
{
  return hashOf(_metadata);
}

bool Custom::satisfiedBy (const ArbiterSelectedVersion &selectedVersion) const
{
  return _predicate(&selectedVersion, _context.get());
}

bool Custom::operator== (const Base &other) const
{
  if (auto *ptr = dynamic_cast<const Custom *>(&other)) {
    return _predicate == ptr->_predicate && _context == ptr->_context;
  } else {
    return false;
  }
}

size_t Custom::hash () const noexcept
{
  return hashOf(_predicate) ^ hashOf(_context);
}

bool Compound::satisfiedBy (const ArbiterSelectedVersion &selectedVersion) const
{
  int minimumPriority = priority();

  for (const auto &requirement : _requirements) {
    // Ignore any requirements which have a higher priority index (i.e., lower
    // priority) than the minimum, because they would normally get discarded
    // during intersection.
    if (requirement->priority() > minimumPriority) {
      continue;
    }

    if (!requirement->satisfiedBy(selectedVersion)) {
      return false;
    }
  }

  return true;
}

std::ostream &Compound::describe (std::ostream &os) const
{
  os << "{ ";

  for (auto it = _requirements.begin(); it != _requirements.end(); ++it) {
    if (it != _requirements.begin()) {
      os << " && ";
    }

    const ArbiterRequirement &requirement = **it;
    os << requirement;
  }

  return os << " }";
}

bool Compound::operator== (const Base &other) const
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

void Compound::visit (Visitor &visitor) const
{
  ArbiterRequirement::visit(visitor);

  for (const auto &requirement : _requirements) {
    requirement->visit(visitor);
  }
}

int Compound::priority () const noexcept
{
  int minimum = std::numeric_limits<int>::max();

  for (const auto &requirement : _requirements) {
    minimum = std::min(minimum, requirement->priority());
  }
  
  return minimum;
}

bool Prioritized::satisfiedBy (const ArbiterSelectedVersion &selectedVersion) const
{
  return _requirement->satisfiedBy(selectedVersion);
}

std::ostream &Prioritized::describe (std::ostream &os) const
{
  return os << *_requirement << " (priority " << _priority << ")";
}

bool Prioritized::operator== (const Arbiter::Base &other) const
{
  if (auto *ptr = dynamic_cast<const Prioritized *>(&other)) {
    return *_requirement == *ptr->_requirement && _priority == ptr->_priority;
  } else {
    return false;
  }
}

size_t Prioritized::hash () const noexcept
{
  return hashOf(*_requirement) ^ hashOf(_priority);
}

void Prioritized::visit (Visitor &visitor) const
{
  ArbiterRequirement::visit(visitor);
  _requirement->visit(visitor);
}

bool ExcludedInstantiation::satisfiedBy (const ArbiterSelectedVersion &selectedVersion) const
{
  const Instantiation::Versions &versions = _excludedInstantiation->_versions;
  return versions.find(selectedVersion) == versions.end();
}

std::ostream &ExcludedInstantiation::describe (std::ostream &os) const
{
  os << "!(";

  const Instantiation::Versions &versions = _excludedInstantiation->_versions;
  for (auto it = versions.begin(); it != versions.end(); ++it) {
    if (it != versions.begin()) {
      os << ", ";
    }

    os << *it;
  }

  return os << ")";
}

bool ExcludedInstantiation::operator== (const Arbiter::Base &other) const
{
  if (auto *ptr = dynamic_cast<const ExcludedInstantiation *>(&other)) {
    return *_excludedInstantiation == *ptr->_excludedInstantiation;
  } else {
    return false;
  }
}

size_t ExcludedInstantiation::hash () const noexcept
{
  return hashOf(*_excludedInstantiation);
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

std::unique_ptr<ArbiterRequirement> Unversioned::intersect (const ArbiterRequirement &rhs) const
{
  return intersectRight<Unversioned>(*this, rhs);
}

std::unique_ptr<ArbiterRequirement> Custom::intersect (const ArbiterRequirement &rhs) const
{
  return intersectRight<Custom>(*this, rhs);
}

std::unique_ptr<ArbiterRequirement> Compound::intersect (const ArbiterRequirement &rhs) const
{
  return intersectRight<Compound>(*this, rhs);
}

std::unique_ptr<ArbiterRequirement> Prioritized::intersect (const ArbiterRequirement &rhs) const
{
  return intersectRight<Prioritized>(*this, rhs);
}

std::unique_ptr<ArbiterRequirement> ExcludedInstantiation::intersect (const ArbiterRequirement &rhs) const
{
  return intersectRight<ExcludedInstantiation>(*this, rhs);
}

} // namespace Requirement
} // namespace Arbiter

using namespace Arbiter;

ArbiterRequirement *ArbiterCreateRequirementAny (void)
{
  return new Requirement::Any;
}

ArbiterRequirement *ArbiterCreateRequirementAtLeast (const ArbiterSemanticVersion *version)
{
  return new Requirement::AtLeast(*version);
}

ArbiterRequirement *ArbiterCreateRequirementCompatibleWith (const ArbiterSemanticVersion *version, ArbiterRequirementStrictness strictness)
{
  return new Requirement::CompatibleWith(*version, strictness);
}

ArbiterRequirement *ArbiterCreateRequirementExactly (const ArbiterSemanticVersion *version)
{
  return new Requirement::Exactly(*version);
}

ArbiterRequirement *ArbiterCreateRequirementUnversioned (ArbiterUserValue metadata)
{
  return new Requirement::Unversioned(Requirement::Unversioned::Metadata(metadata));
}

ArbiterRequirement *ArbiterCreateRequirementCustom (ArbiterRequirementPredicate predicate, ArbiterUserContext context)
{
  return new Requirement::Custom(std::move(predicate), shareUserContext(context));
}

ArbiterRequirement *ArbiterCreateRequirementCompound (const ArbiterRequirement * const *requirements, size_t count)
{
  std::vector<std::shared_ptr<ArbiterRequirement>> vec;
  vec.reserve(count);

  for (size_t i = 0; i < count; i++) {
    vec.emplace_back(requirements[i]->cloneRequirement());
  }

  return new Requirement::Compound(std::move(vec));
}

ArbiterRequirement *ArbiterCreateRequirementPrioritized (const ArbiterRequirement *baseRequirement, int priorityIndex)
{
  return new Arbiter::Requirement::Prioritized(baseRequirement->cloneRequirement(), priorityIndex);
}

bool ArbiterRequirementSatisfiedBy (const ArbiterRequirement *requirement, const ArbiterSelectedVersion *version)
{
  return requirement->satisfiedBy(*version);
}

std::unique_ptr<ArbiterRequirement> ArbiterRequirement::cloneRequirement () const
{
  return std::unique_ptr<ArbiterRequirement>(dynamic_cast<ArbiterRequirement *>(clone().release()));
}

void ArbiterRequirement::visit (Requirement::Visitor &visitor) const
{
  visitor(*this);
}
