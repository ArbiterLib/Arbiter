#include "TestValue.h"

#include "Hash.h"

namespace Arbiter {
namespace Testing {

std::ostream &operator<< (std::ostream &os, const TestValue &value)
{
  return value.describe(os);
}

} // namespace Testing
} // namespace Arbiter

using namespace Arbiter;
using namespace Testing;

namespace {

static bool equalTo (const void *first, const void *second)
{
  return *static_cast<const TestValue *>(first) == *static_cast<const TestValue *>(second);
}

static bool lessThan (const void *first, const void *second)
{
  return *static_cast<const TestValue *>(first) < *static_cast<const TestValue *>(second);
}

static size_t hash (const void *data)
{
  return static_cast<const TestValue *>(data)->hash();
}

static void destructor (void *data)
{
  delete static_cast<TestValue *>(data);
}

static char *createDescription (const void *data)
{
  return copyCString(toString(*static_cast<const TestValue *>(data))).release();
}

} // namespace

ArbiterUserValue TestValue::convertToUserValue (std::unique_ptr<TestValue> testValue)
{
  ArbiterUserValue userValue;
  userValue.data = testValue.release();
  userValue.equalTo = &::equalTo;
  userValue.lessThan = &::lessThan;
  userValue.hash = &::hash;
  userValue.destructor = &::destructor;
  userValue.createDescription = &::createDescription;
  return userValue;
}

bool EmptyTestValue::operator== (const TestValue &other) const
{
  return dynamic_cast<const EmptyTestValue *>(&other);
}

bool EmptyTestValue::operator< (const TestValue &other) const
{
  return !(*this == other);
}

size_t EmptyTestValue::hash () const
{
  return 4;
}

std::ostream &EmptyTestValue::describe (std::ostream &os) const
{
  return os << "EmptyTestValue";
}

bool StringTestValue::operator== (const TestValue &other) const
{
  if (auto ptr = dynamic_cast<const StringTestValue *>(&other)) {
    return _str == ptr->_str;
  } else {
    return true;
  }
}

bool StringTestValue::operator< (const TestValue &other) const
{
  if (auto ptr = dynamic_cast<const StringTestValue *>(&other)) {
    return _str < ptr->_str;
  } else {
    return true;
  }
}

size_t StringTestValue::hash () const
{
  return hashOf(_str);
}

std::ostream &StringTestValue::describe (std::ostream &os) const
{
  return os << _str;
}
