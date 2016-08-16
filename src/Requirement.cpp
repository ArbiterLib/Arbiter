#include "Requirement-inl.h"

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

void ArbiterFreeRequirement (ArbiterRequirement *requirement)
{
  delete requirement;
}

bool ArbiterEqualRequirements (const ArbiterRequirement *lhs, const ArbiterRequirement *rhs)
{
  return *lhs == *rhs;
}

bool ArbiterRequirementSatisfiedBy (const ArbiterRequirement *requirement, const ArbiterSemanticVersion *version)
{
  return requirement->satisfiedBy(*version);
}

namespace Arbiter {
namespace Requirement {

std::ostream &Any::describe (std::ostream &os) const
{
  return os << "(any version)";
}

bool AtLeast::operator== (const ArbiterRequirement &other) const noexcept
{
  if (auto *ptr = dynamic_cast<const AtLeast *>(&other)) {
    return _minimumVersion == ptr->_minimumVersion;
  } else {
    return false;
  }
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

bool CompatibleWith::operator== (const ArbiterRequirement &other) const noexcept
{
  if (auto *ptr = dynamic_cast<const CompatibleWith *>(&other)) {
    return _baseVersion == ptr->_baseVersion;
  } else {
    return false;
  }
}

std::ostream &CompatibleWith::describe (std::ostream &os) const
{
  return os << "~>" << _baseVersion;
}

bool Exactly::operator== (const ArbiterRequirement &other) const noexcept
{
  if (auto *ptr = dynamic_cast<const Exactly *>(&other)) {
    return _version == ptr->_version;
  } else {
    return false;
  }
}

std::ostream &Exactly::describe (std::ostream &os) const
{
  return os << "==" << _version;
}

}
}
