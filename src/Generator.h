#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include "Optional.h"

#include <functional>
#include <iterator>

namespace Arbiter {

/**
 * A lazy generator of values of type T.
 *
 * This class and its Iterator behave a little differently from standard
 * collections, as Generator::begin() and Iterator::operator++() may perform
 * non-trivial work. For this reason, it is important to only call these methods
 * when the next value is definitely desired.
 *
 * Example:
 *
 *  Generator<int> g = …;
 *  for (auto it = g.begin(); it != g.end(); ++it) {
 *    printf("%i", *it);
 *  }
 */
template<typename T>
class Generator final
{
  public:
    using Function = std::function<Optional<T>(void)>;

    /**
     * An input iterator which lazily evaluates a Generator.
     */
    class Iterator final : public std::iterator<std::input_iterator_tag, T>
    {
      public:
        /**
         * Creates an empty iterator.
         */
        Iterator ()
          : _latest()
          , _count(0)
        {}

        Iterator (const Iterator &other)
        {
          copyFrom(other);
        }

        Iterator (Iterator &&other)
        {
          moveFrom(std::move(other));
        }

        Iterator &operator= (const Iterator &other)
        {
          if (this == &other) {
            return *this;
          }

          copyFrom(other);
          return *this;
        }

        Iterator &operator= (Iterator &&other)
        {
          moveFrom(std::move(other));
          return *this;
        }

        /**
         * Evaluates the generator and updates the iterator to the new value.
         */
        Iterator &operator++ ()
        {
          ++_count;
          _latest = _fn();
          return *this;
        }

        /**
         * Evaluates the generator and returns an iterator pointing to the value
         * before the update.
         */
        Iterator operator++ (int)
        {
          Iterator current = *this;
          ++(*this);
          return current;
        }

        bool operator== (const Iterator &other) const
        {
          bool bothEmpty = !_latest && !other._latest;
          return bothEmpty || _count == other._count;
        }

        bool operator!= (const Iterator &other) const
        {
          return !(*this == other);
        }

        /**
         * Returns the latest value yielded from the generator.
         */
        T &operator* () const
        {
          return _latest.value();
        }

        T *operator-> () const
        {
          return _latest.pointer();
        }

        /**
         * Returns whether this iterator has a value (i.e. `operator*` may be
         * invoked).
         */
        operator bool () const
        {
          return static_cast<bool>(_latest);
        }

      private:
        friend class Generator;

        explicit Iterator (const Generator &generator, Optional<T> initial)
          : _fn(generator._fn)
          , _latest(std::move(initial))
        {}

        explicit Iterator (Generator &&generator, Optional<T> initial)
          : _fn(std::move(generator._fn))
          , _latest(std::move(initial))
        {}

        Generator<T>::Function _fn;
        Optional<T> _latest;

        // For == only
        size_t _count{1};

        void copyFrom (const Iterator &other)
        {
          _fn = other._fn;
          _latest = other._latest;
          _count = other._count;
        }

        void moveFrom (Iterator &&other)
        {
          _fn = std::move(other._fn);
          _latest = std::move(other._latest);
          _count = other._count;
        }
    };

    Generator () = delete;

    /**
     * Creates a generator which will invoke the given function to create each
     * successive value.
     *
     * Once the function returns an empty value, the generator is assumed to be
     * exhausted.
     */
    explicit Generator (Function fn)
      : _fn(std::move(fn))
    {}

    Generator (const Generator &other)
      : _fn(other._fn)
    {}

    Generator (Generator &&other)
      : _fn(std::move(other._fn))
    {}

    Generator &operator= (const Generator &other)
    {
      if (this == &other) {
        return *this;
      }

      _fn = other._fn;
      return *this;
    }

    Generator &operator= (Generator &&other)
    {
      _fn = std::move(other._fn);
      return *this;
    }

    /**
     * Evaluates the generator function once, then creates an iterator pointing
     * to that value, or an empty iterator if the generator function returned
     * an empty value.
     */
    Iterator begin () &
    {
      auto initial = _fn();
      return Iterator(*this, std::move(initial));
    }

    /**
     * Evaluates the generator function once, then creates an iterator pointing
     * to that value, or an empty iterator if the generator function returned
     * an empty value.
     */
    Iterator begin () &&
    {
      auto initial = _fn();
      return Iterator(std::move(*this), std::move(initial));
    }

    /**
     * Returns an iterator corresponding to an exhausted generator. This can be
     * used to compare for equality against another iterator and check whether
     * it is valid.
     */
    Iterator end () const
    {
      return Iterator();
    }

  private:
    friend class Iterator;

    Function _fn;

    Optional<T> next ()
    {
      return _fn();
    }
};

/**
 * Creates a Generator which will lazily invoke the given function to yield
 * values.
 *
 * The type of generator is inferred from the return type of the given function.
 */
template<typename Callable>
auto makeGenerator (Callable fn)
{
  return Generator<typename decltype(fn())::Value>(std::move(fn));
}

} // namespace Arbiter
