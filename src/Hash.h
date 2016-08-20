#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <functional>
#include <type_traits>

namespace Arbiter {

/**
 * Hashes an arbitrary value that has an std::hash specialization.
 *
 * This function is slightly more convenient than using std::hash directly,
 * because of type inference.
 */
template<typename T>
size_t hashOf (const T &value)
{
  return std::hash<T>()(value);
}

} // namespace Arbiter
