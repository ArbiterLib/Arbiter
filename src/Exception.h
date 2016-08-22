#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <exception>
#include <stdexcept>

namespace Arbiter {
namespace Exception {

/**
 * Exception type representing an error that was returned from Arbiter client
 * code.
 */
struct UserError final : std::runtime_error
{
  public:
    UserError ()
      : UserError("unspecified user error")
    {}

    explicit UserError (const std::string &string)
      : std::runtime_error(string)
    {}
};

/**
 * Exception type indicating that there were mutually exclusive constraints in
 * a proposed dependency graph.
 */
struct MutuallyExclusiveConstraints final : std::runtime_error
{
  public:
    explicit MutuallyExclusiveConstraints (const std::string &string)
      : std::runtime_error(string)
    {}
};

/**
 * Exception type indicating that there were unsatisfiable constraints for the
 * selected versions in a proposed dependency graph.
 */
struct UnsatisfiableConstraints final : std::runtime_error
{
  public:
    explicit UnsatisfiableConstraints (const std::string &string)
      : std::runtime_error(string)
    {}
};

}
} // namespace Arbiter
