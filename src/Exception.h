#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <exception>
#include <stdexcept>

namespace Arbiter {

/**
 * Exception type representing an error that was returned from Arbiter client
 * code.
 */
struct UserError final : std::runtime_error
{
  UserError ()
    : UserError("unspecified user error")
  {}

  explicit UserError (const std::string &string)
    : std::runtime_error(string)
  {}
};

} // namespace Arbiter
