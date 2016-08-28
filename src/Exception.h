#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <exception>
#include <ostream>
#include <stdexcept>

namespace Arbiter {
namespace Exception {

/**
 * Base type for Arbiter exceptions.
 */
struct Base : std::runtime_error
{
  public:
    Base () = delete;

  protected:
    Base (const std::string &string)
      : std::runtime_error(string)
    {}
};

/**
 * Exception type representing an error that was returned from Arbiter client
 * code.
 */
struct UserError final : Base
{
  public:
    UserError ()
      : UserError("unspecified user error")
    {}

    explicit UserError (const std::string &string)
      : Base(string)
    {}
};

/**
 * Exception type indicating that there were mutually exclusive constraints in
 * a proposed dependency graph.
 */
struct MutuallyExclusiveConstraints final : Base
{
  public:
    explicit MutuallyExclusiveConstraints (const std::string &string)
      : Base(string)
    {}
};

/**
 * Exception type indicating that there were unsatisfiable constraints for the
 * selected versions in a proposed dependency graph.
 */
struct UnsatisfiableConstraints final : Base
{
  public:
    explicit UnsatisfiableConstraints (const std::string &string)
      : Base(string)
    {}
};

}
} // namespace Arbiter

std::ostream &operator<< (std::ostream &os, const Arbiter::Exception::Base &ex);
