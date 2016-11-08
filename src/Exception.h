#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <ostream>
#include <stdexcept>

namespace Arbiter {
namespace Exception {

/**
 * Base type for Arbiter exceptions.
 */
struct Base : public std::runtime_error
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
struct UserError final : public Base
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
struct MutuallyExclusiveConstraints final : public Base
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
struct UnsatisfiableConstraints final : public Base
{
  public:
    explicit UnsatisfiableConstraints (const std::string &string)
      : Base(string)
    {}
};

/**
 * Exception type indicating that an attempt was made to add a node to an
 * ArbiterResolvedDependencyGraph which already existed, but with a conflicting
 * version.
 */
struct ConflictingNode final : public Base
{
  public:
    explicit ConflictingNode (const std::string &string)
      : Base(string)
    {}
};

}
} // namespace Arbiter

std::ostream &operator<< (std::ostream &os, const Arbiter::Exception::Base &ex);
