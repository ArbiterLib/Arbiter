#include "Types.h"

#include "ToString.h"

#include <sstream>
#include <stdexcept>

using namespace Arbiter;

std::ostream &operator<< (std::ostream &os, const Base &object)
{
  return object.describe(os);
}

void ArbiterFree (void *object)
{
  delete static_cast<Base *>(object);
}

void *ArbiterCreateCopy (const void *object)
{
  return static_cast<const Base *>(object)->clone().release();
}

bool ArbiterEqual (const void *left, const void *right)
{
  return *static_cast<const Base *>(left) == *static_cast<const Base *>(right);
}

char *ArbiterCreateDescription (const void *object)
{
  std::stringstream ss;
  ss << *static_cast<const Base *>(object);
  return copyCString(ss.str()).release();
}
