#include "Project.h"

#include "Dependency.h"
#include "Instantiation.h"

#include <algorithm>

namespace Arbiter {

std::shared_ptr<Instantiation> Project::addInstantiation (const ArbiterSelectedVersion &version, const ArbiterDependencyList &dependencyList)
{
  Instantiation::Dependencies dependencies(dependencyList._dependencies.begin(), dependencyList._dependencies.end());

  std::shared_ptr<Instantiation> inst = instantiationForDependencies(dependencies);
  if (!inst) {
    inst = std::make_shared<Instantiation>(std::move(dependencies));
    _instantiations.emplace_back(inst);
  }

  inst->_versions.emplace(version);
  return inst;
}

std::shared_ptr<Instantiation> Project::instantiationForVersion (const ArbiterSelectedVersion &version) const
{
  auto it = std::find_if(_instantiations.begin(), _instantiations.end(), [&](const auto &instantiation) {
    return instantiation->_versions.find(version) != instantiation->_versions.end();
  });

  if (it == _instantiations.end()) {
    return nullptr;
  } else {
    return *it;
  }
}

std::shared_ptr<Instantiation> Project::instantiationForDependencies (const std::unordered_set<ArbiterDependency> &dependencies) const
{
  auto it = std::find_if(_instantiations.begin(), _instantiations.end(), [&](const auto &instantiation) {
    return instantiation->dependencies() == dependencies;
  });

  if (it == _instantiations.end()) {
    return nullptr;
  } else {
    return *it;
  }
}

std::ostream &operator<< (std::ostream &os, const Project &project)
{
  os << "Project domain {";

  for (auto it = project.domain().begin(); it != project.domain().end(); ++it) {
    if (it != project.domain().begin()) {
      os << ", ";
    }

    os << *it;
  }

  os << "}, instantiations {\n";

  for (const auto &instantiation : project.instantiations()) {
    os << "\t" << *instantiation << "\n";
  }
  
  return os << "}";
}

} // namespace Arbiter

ArbiterProjectIdentifier *ArbiterCreateProjectIdentifier (ArbiterUserValue value)
{
  return new ArbiterProjectIdentifier(ArbiterProjectIdentifier::Value(value));
}

const void *ArbiterProjectIdentifierValue (const ArbiterProjectIdentifier *projectIdentifier)
{
  return projectIdentifier->_value.data();
}

std::unique_ptr<Arbiter::Base> ArbiterProjectIdentifier::clone () const
{
  return std::make_unique<ArbiterProjectIdentifier>(*this);
}

std::ostream &ArbiterProjectIdentifier::describe (std::ostream &os) const
{
  return os << "ArbiterProjectIdentifier(" << _value << ")";
}

bool ArbiterProjectIdentifier::operator== (const Arbiter::Base &other) const
{
  auto ptr = dynamic_cast<const ArbiterProjectIdentifier *>(&other);
  if (!ptr) {
    return false;
  }

  return _value == ptr->_value;
}

size_t std::hash<ArbiterProjectIdentifier>::operator() (const ArbiterProjectIdentifier &project) const
{
  return hashOf(project._value);
}
