#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include <iterator>
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

template<typename It>
struct MultipassIterator final
{
  public:
    It _begin;
    It _current;
    It _end;

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

    PermutationIterator &operator++ ()
    {
      if (_rightCurrent == _rightEnd) {
        _rightCurrent = _rightBegin;

        assert(_leftCurrent != _leftEnd);
        ++_leftCurrent;
      } else {
        ++_rightCurrent;
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
      return _leftCurrent == other._leftCurrent && _rightCurrent == other._rightCurrent;
    }

    bool operator!= (const PermutationIterator &other) const
    {
      return !(*this == other);
    }

    TupledIteratorReference<It...> operator*() const
    {
      return std::make_tuple(*_leftCurrent, *_rightCurrent);
    }

    /**
     * Returns whether the iterator is valid (i.e., dereferenceable).
     *
     * If this is false, it is an error to increment the iterator further.
     */
    operator bool () const
    {
      return _leftCurrent != _leftEnd && _rightCurrent != _rightEnd;
    }

  private:
    std::tuple<MultipassIterator<It>...> _iterators;
};

/**
 * Creates an iterator which generates every possible combination of the values
 * between the given ranges.
 *
 * All inputs must be forward iterators.
 */
template<typename LeftIt, typename RightIt>
PermutationIterator<LeftIt, RightIt> permute (LeftIt leftBegin, LeftIt leftEnd, RightIt rightBegin, RightIt rightEnd)
{
  return PermutationIterator<LeftIt, RightIt>(std::move(leftBegin), std::move(leftEnd), std::move(rightBegin), std::move(rightEnd));
}

} // namespace Arbiter
