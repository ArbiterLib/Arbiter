#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <type_traits>

namespace Arbiter {

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

/**
 * Creates an IteratorRange encompassing the entirety of the given collection.
 */
template<typename Collection>
auto makeIteratorRange (Collection &collection)
{
  auto begin = std::begin(collection);
  return IteratorRange<decltype(begin)>(std::move(begin), std::end(collection));
}

/**
 * Creates an IteratorRange encompassing the entirety of the given read-only
 * collection.
 */
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

    MultipassIterator &operator++ ()
    {
      ++_current;
      return *this;
    }

    void reset ()
    {
      _current = _begin;
    }

    typename std::iterator_traits<It>::reference operator* () const
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
 * The result type of an iterator which yields dynamically-created vectors.
 */
template<typename It>
using IteratorResultVector = std::vector<typename std::iterator_traits<It>::value_type>;

/**
 * An iterator which generates every possible combination of the values of other
 * iterators.
 *
 * The input type must be a forward iterator.
 */
template<typename It>
class PermutationIterator final : public std::iterator<std::forward_iterator_tag, IteratorResultVector<It>, std::ptrdiff_t, IteratorResultVector<It>, IteratorResultVector<It>>
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
    PermutationIterator (std::vector<IteratorRange<It>> ranges)
    {
      _iterators.reserve(ranges.size());

      for (IteratorRange<It> &range : ranges) {
        _iterators.emplace_back(std::move(range));
      }
    }

    PermutationIterator &operator++ ()
    {
      assert(static_cast<bool>(*this));

      for (auto it = _iterators.rbegin(); it != _iterators.rend(); ++it) {
        MultipassIterator<It> &multipass = *it;
        ++multipass;

        if (multipass) {
          return *this;
        }

        // Don't reset the first iterator, as that's how we'll detect validity.
        if (_iterators.rend() - it > 1) {
          multipass.reset();
        }
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

    IteratorResultVector<It> operator* () const
    {
      assert(static_cast<bool>(*this));

      IteratorResultVector<It> values;
      values.reserve(_iterators.size());

      for (const MultipassIterator<It> &multipass : _iterators) {
        values.emplace_back(*multipass);
      }

      return values;
    }

    /**
     * Returns whether the iterator is valid (i.e., dereferenceable).
     *
     * If this is false, it is an error to increment the iterator further.
     */
    explicit operator bool () const
    {
      return !_iterators.empty() && _iterators.at(0);
    }

  private:
    std::vector<MultipassIterator<It>> _iterators;
};

} // namespace Arbiter
