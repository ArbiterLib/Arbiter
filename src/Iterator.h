#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include "Tuple.h"

#include <iterator>
#include <stdexcept>
#include <tuple>

namespace Arbiter {

/**
 * The value type for an iterator which combines the values of other iterators
 * into a tuple.
 */
template<typename... It>
using TupledIteratorValue = std::tuple<typename std::iterator_traits<It...>::value_type>;

/**
 * The pointer type for an iterator which combines the values of other iterators
 * into a tuple.
 */
template<typename... It>
using TupledIteratorPointer = std::tuple<typename std::iterator_traits<It...>::pointer>;

/**
 * The reference type for an iterator which combines the values of other
 * iterators into a tuple.
 */
template<typename... It>
using TupledIteratorReference = std::tuple<typename std::iterator_traits<It...>::reference>;

/**
 * The base class for an iterator which combines the values of other iterators
 * into a tuple.
 */
template<typename Category, typename Distance, typename... It>
using TupledIterator = std::iterator<Category, TupledIteratorValue<It...>, Distance, TupledIteratorPointer<It...>, TupledIteratorReference<It...>>;

/**
 * Represents a pair of iterators bracketing the start and end of a range.
 */
template<typename It>
struct IteratorRange final
{
  public:
    It _begin;
    It _end;

    IteratorRange (It begin, It end)
      : _begin(std::move(begin))
      , _end(std::move(end))
    {}
};

template<typename Collection>
auto makeIteratorRange (Collection &collection)
{
  auto begin = std::begin(collection);
  return IteratorRange<decltype(begin)>(std::move(begin), std::end(collection));
}

template<typename Collection>
auto makeIteratorRange (const Collection &collection)
{
  auto begin = std::begin(collection);
  return IteratorRange<decltype(begin)>(std::move(begin), std::end(collection));
}

/**
 * Contains the state necessary to perform an algorithm over an iterator
 * multiple times.
 */
template<typename It>
class MultipassIterator final
{
  public:
    MultipassIterator () = default;

    MultipassIterator (It begin, It current, It end)
      : _begin(std::move(begin))
      , _current(std::move(current))
      , _end(std::move(end))
    {}

    explicit MultipassIterator (const IteratorRange<It> &range)
      : _begin(range._begin)
      , _current(range._begin)
      , _end(range._end)
    {}

    explicit MultipassIterator (IteratorRange<It> &&range)
      : _begin(std::move(range._begin))
      , _end(std::move(range._end))
    {
      _current = _begin;
    }

    explicit operator bool () const
    {
      return _current != _end;
    }

    bool incrementIfValid ()
    {
      if (static_cast<bool>(*this)) {
        ++_current;
        return true;
      } else {
        return false;
      }
    }

    void reset ()
    {
      _current = _begin;
    }

    std::iterator_traits<It>::reference operator* () const
    {
      assert(static_cast<bool>(*this));
      return *_current;
    }

  private:
    It _begin;
    It _current;
    It _end;

};

/**
 * An iterator which generates every possible combination of the values of other
 * iterators.
 *
 * All input types must refer to forward iterators.
 */
template<typename... It>
class PermutationIterator final : public TupledIterator<std::forward_iterator_tag, std::ptrdiff_t, It...>
{
  public:
    /**
     * Creates an empty iterator. The iterator must not be dereferenced.
     */
    PermutationIterator ()
    {}

    /**
     * Creates an iterator which will create all possible combinations between
     * the given ranges.
     */
    PermutationIterator (IteratorRange<It>... ranges)
      : _iterators(std::make_tuple(MultipassIterator<It>(std::move(ranges))...))
    {}

    PermutationIterator &operator++ () noexcept(false)
    {
      bool incremented = Tuple::foldr(_iterators, false, &incrementLastValid, std::make_index_sequence<sizeof...(It)>());
      if (!incremented) {
        throw std::out_of_range("Attempt to increment PermutationIterator beyond end");
      }

      return *this;
    }

    PermutationIterator operator++ (int)
    {
      auto iter = *this;
      ++(*this);
      return iter;
    }

    bool operator== (const PermutationIterator &other) const
    {
      return _iterators == other._iterators;
    }

    bool operator!= (const PermutationIterator &other) const
    {
      return !(*this == other);
    }

    TupledIteratorReference<It...> operator* () const
    {
      return Tuple::multiDereference(_iterators, std::make_index_sequence<sizeof...(It)>());
    }

    /**
     * Returns whether the iterator is valid (i.e., dereferenceable).
     *
     * If this is false, it is an error to increment the iterator further.
     */
    explicit operator bool () const
    {
      return Tuple::multiAnd(_iterators, std::make_index_sequence<sizeof...(It)>());
    }

  private:
    using IteratorTuple = std::tuple<MultipassIterator<It>...>;

    IteratorTuple _iterators;

    template<typename InnerIt>
    static bool incrementLastValid (MultipassIterator<InnerIt> &it, bool incrementedNext)
    {
      if (incrementedNext) {
        return true;
      }

      if (it.incrementIfValid()) {
        return true;
      } else {
        it.reset();
        return false;
      }
    }
};

/**
 * Creates an iterator which generates every possible combination of the values
 * between the given ranges.
 *
 * All inputs must be forward iterators or better.
 */
template<typename... It>
PermutationIterator<It...> permute (IteratorRange<It>... ranges)
{
  return PermutationIterator<It...>(std::move(ranges)...);
}

/**
 * Creates an iterator which generates every possible combination of the values
 * between the given collections.
 *
 * The given collections must support multipass iteration.
 */
template<typename... Collections>
PermutationIterator<It...> permute (Collections... &&collections)
{
  return permute(makeIteratorRange(std::forward<Collections>(collections))...)
}

} // namespace Arbiter
