#include "Dependency.h"

#include "Hash.h"
#include "Requirement.h"

#include <algorithm>

using namespace Arbiter;

ArbiterDependency *ArbiterCreateDependency (const ArbiterProjectIdentifier *projectIdentifier, const ArbiterRequirement *requirement)
{
  return new ArbiterDependency(*projectIdentifier, *requirement);
}

const ArbiterProjectIdentifier *ArbiterDependencyProject (const ArbiterDependency *dependency)
{
  return &dependency->_projectIdentifier;
}

const ArbiterRequirement *ArbiterDependencyRequirement (const ArbiterDependency *dependency)
{
  return &dependency->requirement();
}

ArbiterDependencyList *ArbiterCreateDependencyList (const ArbiterDependency * const *dependencies, size_t count)
{
  std::vector<ArbiterDependency> vec;
  vec.reserve(count);

  for (size_t i = 0; i < count; i++) {
    vec.emplace_back(*dependencies[i]);
  }

  return new ArbiterDependencyList(std::move(vec));
}

ArbiterResolvedDependency *ArbiterCreateResolvedDependency (const ArbiterProjectIdentifier *project, const ArbiterSelectedVersion *version)
{
  return new ArbiterResolvedDependency(*project, *version);
}

const ArbiterProjectIdentifier *ArbiterResolvedDependencyProject (const ArbiterResolvedDependency *dependency)
{
  return &dependency->_project;
}

const ArbiterSelectedVersion *ArbiterResolvedDependencyVersion (const ArbiterResolvedDependency *dependency)
{
  return &dependency->_version;
}

ArbiterDependency::ArbiterDependency (ArbiterProjectIdentifier projectIdentifier, const ArbiterRequirement &requirement)
  : _projectIdentifier(std::move(projectIdentifier))
  , _requirement(requirement.cloneRequirement())
{}

ArbiterDependency &ArbiterDependency::operator= (const ArbiterDependency &other)
{
  if (this == &other) {
    return *this;
  }

  _projectIdentifier = other._projectIdentifier;
  _requirement = other.requirement().cloneRequirement();
  return *this;
}

std::unique_ptr<Arbiter::Base> ArbiterDependency::clone () const
{
  return std::make_unique<ArbiterDependency>(*this);
}

std::ostream &ArbiterDependency::describe (std::ostream &os) const
{
  return os << "Dependency(" << _projectIdentifier << requirement() << ")";
}

bool ArbiterDependency::operator== (const Base &other) const
{
  auto ptr = dynamic_cast<const ArbiterDependency *>(&other);
  if (!ptr) {
    return false;
  }

  return _projectIdentifier == ptr->_projectIdentifier && *_requirement == *(ptr->_requirement);
}

std::unique_ptr<Arbiter::Base> ArbiterDependencyList::clone () const
{
  return std::make_unique<ArbiterDependencyList>(*this);
}

std::ostream &ArbiterDependencyList::describe (std::ostream &os) const
{
  os << "Dependency list:";

  for (const auto &dep : _dependencies) {
    os << "\n" << dep;
  }

  return os;
}

bool ArbiterDependencyList::operator== (const Arbiter::Base &other) const
{
  auto ptr = dynamic_cast<const ArbiterDependencyList *>(&other);
  if (!ptr) {
    return false;
  }

  return _dependencies == ptr->_dependencies;
}

std::unique_ptr<Arbiter::Base> ArbiterResolvedDependency::clone () const
{
  return std::make_unique<ArbiterResolvedDependency>(*this);
}

std::ostream &ArbiterResolvedDependency::describe (std::ostream &os) const
{
  return os << _project << " @ " << _version;
}

bool ArbiterResolvedDependency::operator== (const Arbiter::Base &other) const
{
  auto ptr = dynamic_cast<const ArbiterResolvedDependency *>(&other);
  if (!ptr) {
    return false;
  }

  return _project == ptr->_project && _version == ptr->_version;
}

bool ArbiterResolvedDependency::operator< (const ArbiterResolvedDependency &other) const
{
  return _project < other._project;
}

size_t std::hash<ArbiterDependency>::operator() (const ArbiterDependency &dependency) const
{
  return hashOf(dependency._projectIdentifier)
    ^ hashOf(dependency.requirement());
}

size_t std::hash<ArbiterResolvedDependency>::operator() (const ArbiterResolvedDependency &dependency) const
{
  return hashOf(dependency._project)
    ^ hashOf(dependency._version);
}
