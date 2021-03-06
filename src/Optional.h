#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include "Hash.h"

#include <cassert>
#include <functional>
#include <ostream>
#include <type_traits>

namespace Arbiter {

/**
 * Sentinel value which can be used to create an empty Optional of any type.
 */
struct None final
{};

/**
 * Represents a value which may or may not exist.
 */
template<typename T>
struct Optional final
{
  public:
    using Value = T;

    /**
     * Creates an empty Optional.
     */
    constexpr Optional () noexcept
      : _hasValue(false)
    {}

    /**
     * Creates an empty Optional.
     */
    Optional (const None &) noexcept
      : _hasValue(false)
    {}

    /**
     * Creates an Optional containing a copy of the given value.
     */
    Optional (const T &value) noexcept(std::is_nothrow_copy_constructible<T>::value)
      : _hasValue(true)
    {
      construct(value);
    }

    /**
     * Creates an Optional containing the given value.
     */
    Optional (T &&value) noexcept(std::is_nothrow_move_constructible<T>::value)
      : _hasValue(true)
    {
      construct(std::move(value));
    }

    Optional (const Optional &other) noexcept(std::is_nothrow_copy_constructible<T>::value)
      : _hasValue(other._hasValue)
    {
      if (_hasValue) {
        construct(other.value());
      }
    }

    /**
     * Converts from another Optional containing a compatible type.
     */
    template<typename U>
    Optional (const Optional<U> &other) noexcept(std::is_nothrow_constructible<T, const U &>::value)
      : _hasValue(static_cast<bool>(other))
    {
      if (_hasValue) {
        construct(other.value());
      }
    }

    Optional &operator= (const Optional &other) noexcept(std::is_nothrow_destructible<T>::value && std::is_nothrow_copy_constructible<T>::value)
    {
      if (&other == this) {
        return *this;
      }

      destroyIfNeeded();

      if ((_hasValue = other._hasValue)) {
        construct(other.value());
      }

      return *this;
    }

    Optional (Optional &&other) noexcept(std::is_nothrow_move_constructible<T>::value)
      : _hasValue(other._hasValue)
    {
      if (_hasValue) {
        construct(std::move(other.value()));
      }
    }

    /**
     * Converts from another Optional containing a compatible type.
     */
    template<typename U>
    Optional (Optional<U> &&other) noexcept(std::is_nothrow_constructible<T, U &&>::value)
      : _hasValue(static_cast<bool>(other))
    {
      if (_hasValue) {
        construct(std::move(other.value()));
      }
    }

    Optional &operator= (Optional &&other) noexcept(std::is_nothrow_destructible<T>::value && std::is_nothrow_move_constructible<T>::value)
    {
      destroyIfNeeded();

      if ((_hasValue = other._hasValue)) {
        construct(std::move(other.value()));
      }

      return *this;
    }

    ~Optional () noexcept(std::is_nothrow_destructible<T>::value)
    {
      destroyIfNeeded();
    }

    /**
     * Creates an Optional from a pointer which may or may not be null.
     */
    static Optional fromPointer (const T *pointer) noexcept(std::is_nothrow_copy_constructible<T>::value)
    {
      if (pointer) {
        return Optional(*pointer);
      } else {
        return Optional();
      }
    }

    /**
     * Returns true if this Optional contains a value, or false if it is empty.
     */
    explicit operator bool () const noexcept
    {
      return _hasValue;
    }

    /**
     * Accesses the value in the Optional.
     *
     * The Optional must not be empty.
     */
    T &value () noexcept
    {
      assert(_hasValue);
      return *reinterpret_cast<T *>(&_storage);
    }

    /**
     * Accesses the value in the Optional.
     *
     * The Optional must not be empty.
     */
    const T &value () const noexcept
    {
      assert(_hasValue);
      return *reinterpret_cast<const T *>(&_storage);
    }

    /**
     * Returns a pointer to the value in the Optional, or `nullptr` if empty.
     */
    T *pointer () noexcept
    {
      return _hasValue ? reinterpret_cast<T *>(&_storage) : nullptr;
    }

    /**
     * Returns a pointer to the value in the Optional, or `nullptr` if empty.
     */
    const T *pointer () const noexcept
    {
      return _hasValue ? reinterpret_cast<const T *>(&_storage) : nullptr;
    }

    T &operator* () noexcept
    {
      return value();
    }

    const T &operator* () const noexcept
    {
      return value();
    }

    T *operator-> () noexcept
    {
      return pointer();
    }

    const T *operator-> () const noexcept
    {
      return pointer();
    }

  private:
    std::aligned_storage_t<sizeof(T), alignof(T)> _storage;
    bool _hasValue;

    void destroyIfNeeded () noexcept(std::is_nothrow_destructible<T>::value)
    {
      if (_hasValue) {
        value().~T();
      }
    }

    void construct (const T &value) noexcept(std::is_nothrow_copy_constructible<T>::value)
    {
      new(&_storage) T(value);
    }

    void construct (T &&value) noexcept(std::is_nothrow_move_constructible<T>::value)
    {
      new(&_storage) T(std::move(value));
    }
};

template<typename T>
bool operator== (const Optional<T> &lhs, const Optional<T> &rhs)
{
  if (lhs) {
    return rhs && lhs.value() == rhs.value();
  } else {
    return !rhs;
  }
}

/**
 * Creates an Optional from a value, inferring its type.
 */
template<typename T>
auto makeOptional (T &&value)
{
  return Optional<typename std::decay<T>::type>(std::forward<T>(value));
}

} // namespace Arbiter

namespace std {

template<typename T>
struct hash<Arbiter::Optional<T>> final
{
  public:
    size_t operator() (const Arbiter::Optional<T> &optional) const
    {
      if (optional) {
        return Arbiter::hashOf(optional.value());
      } else {
        return 0;
      }
    }
};

template<typename T>
std::ostream &operator<< (std::ostream &os, const Arbiter::Optional<T> &optional)
{
  if (optional) {
    return os << "Optional(" << *optional << ")";
  } else {
    return os << "None";
  }
}

} // namespace std
