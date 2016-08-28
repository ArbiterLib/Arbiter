#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <tuple>
#include <utility>

namespace Arbiter {
namespace Tuple {

/**
 * Dereferences multiple values within a given tuple, identified by the indices
 * specified.
 *
 * Returns a new tuple containing the results of dereferencing those values.
 */
template<typename Tuple, size_t... Indices>
static auto multiDereference (Tuple &&tuple, std::index_sequence<Indices...>)
{
  return std::make_tuple(*std::get<Indices>(std::forward<Tuple>(tuple))...);
}

template<typename Tuple>
static bool multiAnd (const Tuple &, std::index_sequence<>)
{
  return true;
}

/**
 * Performs logical AND across multiple indices of a tuple. Short-circuits when
 * possible.
 */
template<typename Tuple, size_t NextIndex, size_t... Remaining>
static bool multiAnd (const Tuple &tuple, std::index_sequence<NextIndex, Remaining...>)
{
  return static_cast<bool>(std::get<NextIndex>(tuple)) && multiAnd(tuple, std::index_sequence<Remaining...>());
}

template<typename Tuple, typename Result, typename Transform>
static Result foldr (Tuple &&, Result initial, Transform &&, std::index_sequence<>)
{
  return initial;
}

/**
 * Performs a right-associative fold over a tuple.
 */
template<typename Tuple, typename Result, typename Transform, size_t NextIndex, size_t... Remaining>
static Result foldr (Tuple &&tuple, Result initial, Transform &&transform, std::index_sequence<NextIndex, Remaining...>)
{
  Result right = foldr(std::forward<Tuple>(tuple), std::move(initial), std::forward<Transform>(transform), std::index_sequence<Remaining...>());
  return transform(std::get<NextIndex>(std::forward<Tuple>(tuple)), std::move(right));
}

} // namespace Tuple
} // namespace Arbiter
