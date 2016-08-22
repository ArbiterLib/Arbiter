#include "Dependency.h"

#include "Hash.h"
#include "Requirement.h"

using namespace Arbiter;

size_t std::hash<ArbiterDependency>::operator() (const ArbiterDependency &dependency) const
{
  return hashOf(dependency._projectIdentifier)
    ^ hashOf(dependency.requirement());
}

ArbiterProjectIdentifier *ArbiterCreateProjectIdentifier (ArbiterUserValue value)
{
  return new ArbiterProjectIdentifier(ArbiterProjectIdentifier::Value(value));
}

const void *ArbiterProjectIdentifierValue (const ArbiterProjectIdentifier *projectIdentifier)
{
  return projectIdentifier->_value.data();
}

void ArbiterFreeProjectIdentifier (ArbiterProjectIdentifier *projectIdentifier)
{
  delete projectIdentifier;
}

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

void ArbiterFreeDependency (ArbiterDependency *dependency)
{
  delete dependency;
}

ArbiterDependencyList *ArbiterCreateDependencyList (const ArbiterDependency *dependencies, size_t count)
{
  return new ArbiterDependencyList(std::vector<ArbiterDependency>(dependencies, dependencies + count));
}

void ArbiterFreeDependencyList (ArbiterDependencyList *dependencyList)
{
  delete dependencyList;
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

void ArbiterFreeResolvedDependency (ArbiterResolvedDependency *dependency)
{
  delete dependency;
}

std::ostream &operator<< (std::ostream &os, const ArbiterProjectIdentifier &identifier)
{
  return os << "ArbiterProjectIdentifier(" << identifier._value << ")";
}

ArbiterDependency::ArbiterDependency (ArbiterProjectIdentifier projectIdentifier, const ArbiterRequirement &requirement)
  : _projectIdentifier(std::move(projectIdentifier))
  , _requirement(requirement.clone())
{}

ArbiterDependency &ArbiterDependency::operator= (const ArbiterDependency &other)
{
  if (this == &other) {
    return *this;
  }

  _projectIdentifier = other._projectIdentifier;
  _requirement = other.requirement().clone();
  return *this;
}

bool ArbiterDependency::operator== (const ArbiterDependency &other) const
{
  return _projectIdentifier == other._projectIdentifier && *_requirement == *(other._requirement);
}

std::ostream &operator<< (std::ostream &os, const ArbiterDependency &dependency)
{
  return os << "Dependency(" << dependency._projectIdentifier << dependency.requirement() << ")";
}

std::ostream &operator<< (std::ostream &os, const ArbiterDependencyList &dependencyList)
{
  os << "Dependency list:";

  for (const auto &dep : dependencyList._dependencies) {
    os << "\n" << dep;
  }

  return os;
}

std::ostream &operator<< (std::ostream &os, const ArbiterResolvedDependency &dependency)
{
  return os << dependency._project << " @ " << dependency._version;
}
