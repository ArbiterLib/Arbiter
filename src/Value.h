#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <arbiter/Value.h>

#include "ToString.h"

#include <cassert>
#include <ostream>

namespace Arbiter {

/**
 * Expresses shared ownership of opaque user-provided data, which was originally
 * described in an ArbiterUserValue.
 *
 * `Owner` is a phantom type used to associate the SharedUserValue with its
 * usage in a particular class. This helps prevent two SharedUserValue instances
 * from being compared if they represent conceptually different things (which
 * might crash user code).
 */
template<typename Owner>
class SharedUserValue final
{
  public:
    SharedUserValue ()
    {}

    explicit SharedUserValue (ArbiterUserValue value)
      : _data(std::shared_ptr<void>(value.data, (value.destructor ? value.destructor : &noOpDestructor)))
      , _equalTo(value.equalTo)
      , _lessThan(value.lessThan)
      , _createDescription(value.createDescription)
    {
      assert(_equalTo);
      assert(_lessThan);
    }

    bool operator== (const SharedUserValue &other) const
    {
      assert(_equalTo == other._equalTo);
      return _equalTo(data(), other.data());
    }

    bool operator!= (const SharedUserValue &other) const
    {
      return !(*this == other);
    }

    bool operator< (const SharedUserValue &other) const
    {
      assert(_lessThan == other._lessThan);
      return _lessThan(data(), other.data());
    }

    bool operator> (const SharedUserValue &other) const
    {
      return other < *this;
    }

    bool operator>= (const SharedUserValue &other) const
    {
      return !(*this < other);
    }

    bool operator<= (const SharedUserValue &other) const
    {
      return !(*this > other);
    }

    void *data () noexcept
    {
      return _data.get();
    }

    const void *data () const noexcept
    {
      return _data.get();
    }

    std::string description () const
    {
      if (_createDescription) {
        return Arbiter::copyAcquireCString(_createDescription(data()));
      } else {
        return "Arbiter::SharedUserValue";
      }
    }

  private:
    std::shared_ptr<void> _data;
    bool (*_equalTo)(const void *first, const void *second);
    bool (*_lessThan)(const void *first, const void *second);
    char *(*_createDescription)(const void *data);

    static void noOpDestructor (void *)
    {}
};

} // namespace Arbiter

template<typename Owner>
std::ostream &operator<< (std::ostream &os, const Arbiter::SharedUserValue<Owner> &value)
{
  return os << value.description();
}
