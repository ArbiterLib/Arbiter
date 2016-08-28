#include "Exception.h"

using namespace Arbiter;
using namespace Exception;

std::ostream &operator<< (std::ostream &os, const Base &ex)
{
  return os << ex.what();
}
