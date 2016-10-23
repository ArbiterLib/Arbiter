#include "Project.h"

#include "Dependency.h"
#include "Instantiation.h"

#include <algorithm>

namespace Arbiter {

void Project::addInstantiation (const std::shared_ptr<Instantiation> &instantiation)
{
  // TODO: Check for duplicates?

  _instantiations.emplace_back(instantiation);
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

} // namespace Arbiter
