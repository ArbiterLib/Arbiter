#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <cstdlib>
#include <memory>
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

/**
 * Returns a unique_ptr which wraps a NUL-terminated copy of the `c_str()` of
 * the given string.
 */
std::unique_ptr<char[]> copyCString (const std::string &str);

/**
 * Returns a unique_ptr which takes ownership of `str`.
 */
std::unique_ptr<char[], decltype(&free)> acquireCString (char *str);

/**
 * Returns a string which is a copy of `str`, which will then be freed.
 */
std::string copyAcquireCString (char *str);

} // namespace Arbiter
