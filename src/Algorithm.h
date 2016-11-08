#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include "Optional.h"

#include <algorithm>
#include <type_traits>

namespace Arbiter {

/**
 * Resets a swappable value to its default constructor.
 */
template<typename Swappable>
void reset (Swappable &value)
{
  Swappable temp;
  std::swap(value, temp);
}

} // namespace Arbiter
