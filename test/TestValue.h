#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include "Value.h"

#include <memory>
#include <ostream>
#include <string>

namespace Arbiter {
namespace Testing {

/**
 * A C++ object capable of being an ArbiterUserValue.
 */
class TestValue
{
  public:
    virtual ~TestValue () = default;

    virtual bool operator== (const TestValue &) const = 0;
    virtual bool operator< (const TestValue &) const = 0;
    virtual std::ostream &describe (std::ostream &os) const = 0;
    virtual size_t hash () const = 0;

    static ArbiterUserValue convertToUserValue (std::unique_ptr<TestValue> testValue);
};

std::ostream &operator<< (std::ostream &os, const TestValue &value);

class EmptyTestValue final : public TestValue
{
  public:
    EmptyTestValue () = default;

    bool operator== (const TestValue &other) const override;
    bool operator< (const TestValue &other) const override;
    size_t hash () const override;
    std::ostream &describe (std::ostream &os) const override;
};

struct StringTestValue final : public TestValue
{
  public:
    explicit StringTestValue (std::string str)
      : _str(std::move(str))
    {}

    bool operator== (const TestValue &other) const override;
    bool operator< (const TestValue &other) const override;
    size_t hash () const override;
    std::ostream &describe (std::ostream &os) const override;

  private:
    std::string _str;
};

template<typename Owner, typename Value, typename... Args>
SharedUserValue<Owner> makeSharedUserValue (Args &&...args)
{
  return SharedUserValue<Owner>(TestValue::convertToUserValue(std::make_unique<Value>(std::forward<Args>(args)...)));
}

} // namespace Testing
} // namespace Arbiter
