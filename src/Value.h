#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <arbiter/Value.h>

#include <cassert>
#include <ostream>

namespace Arbiter {

class SharedUserValue
{
  public:
    explicit SharedUserValue (ArbiterUserValue value)
      : _data(std::shared_ptr<void>(value.data, (value.destructor ? value.destructor : &noOpDestructor)))
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
        std::unique_ptr<char, FreeDeleter> str(_createDescription(data()));
        return std::string(str.get());
      } else {
        return "Arbiter::SharedUserValue";
      }
    }

  private:
    std::shared_ptr<void> _data;
    bool (*_equals)(const void *first, const void *second);
    char *(*_createDescription)(const void *data);

    struct FreeDeleter
    {
      void operator() (void *ptr) const
      {
        free(ptr);
      }
    };

    static void noOpDestructor (void *)
    {}
};

} // namespace Arbiter

std::ostream &operator<< (std::ostream &os, const Arbiter::SharedUserValue &value);
