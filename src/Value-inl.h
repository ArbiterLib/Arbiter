#ifndef ARBITER_VALUE_INL_H
#define ARBITER_VALUE_INL_H

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include "Value.h"

#include <cassert>
#include <ostream>

namespace Arbiter {

class SharedUserValue
{
  public:
    explicit SharedUserValue (ArbiterUserValue value)
      : _data(std::shared_ptr<void>(value.data, value.destructor))
      , _equals(value.equals)
      , _createDescription(value.createDescription)
    {
      assert(_equals);
    }

    bool operator== (const SharedUserValue &other) const
    {
      return _equals(data(), other.data());
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
        return _createDescription(data());
      } else {
        return "Arbiter::SharedUserValue";
      }
    }

  private:
    std::shared_ptr<void> _data;
    bool (*_equals)(const void *first, const void *second);
    char *(*_createDescription)(const void *data);
};

std::ostream &operator<< (std::ostream &os, const SharedUserValue &value)
{
  return os << value.description();
}

}

#endif
