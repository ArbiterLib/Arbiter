#include "Instantiation.h"

#include "Hash.h"
#include "Requirement.h"

#include <algorithm>

namespace Arbiter {

bool Instantiation::satisfies (const ArbiterRequirement &requirement) const
{
  return std::all_of(_versions.begin(), _versions.end(), [&](const ArbiterSelectedVersion &version) {
    // If the requirement excludes this instantiation, we'll find out on the
    // first version element, short-circuiting the rest of the enumeration.
    return requirement.satisfiedBy(version);
  });
}

bool Instantiation::operator== (const Instantiation &other) const
{
  return _dependencies == other._dependencies;
}

std::ostream &operator<< (std::ostream &os, const Instantiation &instantiation)
{
  os << "Instantiation {";
  for (auto it = instantiation._versions.begin(); it != instantiation._versions.end(); ++it) {
    if (it != instantiation._versions.begin()) {
      os << ", ";
    }

    os << *it;
  }

  return os << "}";
}

} // namespace Arbiter

namespace std {

size_t std::hash<Arbiter::Instantiation>::operator() (const Arbiter::Instantiation &value) const
{
  size_t h = Arbiter::hashOf(value.dependencies().size());

  if (!value.dependencies().empty()) {
    h ^= Arbiter::hashOf(*value.dependencies().begin());
  }

  return h;
}

} // namespace std
