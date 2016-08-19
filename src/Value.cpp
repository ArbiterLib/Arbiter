#include "Value.h"

using namespace Arbiter;

std::ostream &operator<< (std::ostream &os, const SharedUserValue &value)
{
  return os << value.description();
}
