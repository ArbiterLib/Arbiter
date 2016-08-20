#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <sstream>
#include <string>

namespace Arbiter {

/**
 * Converts an arbitrary value into a string, using operator<<.
 */
template<typename T>
std::string toString (const T &value)
{
  std::stringstream ss;
  ss << value;
  return ss.str();
}

} // namespace Arbiter
