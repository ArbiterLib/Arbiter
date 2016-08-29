#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <arbiter/Types.h>

#include <memory>
#include <ostream>

namespace Arbiter {

/**
 * Base class for public-facing Arbiter types, such that they automatically get
 * freeing, cloning, equality, and description functionality.
 */
class Base
{
  public:
    virtual ~Base () noexcept(false)
    {}

    virtual std::unique_ptr<Base> clone () const = 0;
    virtual std::ostream &describe (std::ostream &os) const = 0;
    virtual bool operator== (const Base &other) const = 0;

    bool operator!= (const Base &other) const
    {
      return !(*this == other);
    }
};

} // namespace Arbiter

std::ostream &operator<< (std::ostream &os, const Arbiter::Base &object);
