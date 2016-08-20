#include "Dependency.h"

using namespace Arbiter;

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

std::ostream &operator<< (std::ostream &os, const ArbiterProjectIdentifier &identifier)
{
  return os << "ArbiterProjectIdentifier(" << identifier._value << ")";
}

ArbiterDependency &ArbiterDependency::operator= (const ArbiterDependency &other)
{
  if (this == &other) {
    return *this;
  }

  _projectIdentifier = other._projectIdentifier;
  _requirement = other.requirement().clone();
  return *this;
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
