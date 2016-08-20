#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include "Hash.h"

#include <type_traits>

namespace Arbiter {

/**
 * Temporarily holds the left side of an Either, with an unbound Right type. 
 */
template<typename T>
struct Left final
{
  T _value;

  Left () = delete;

  Left (const T &value) noexcept(std::is_nothrow_copy_constructible<T>::value)
    : _value(value)
  {}

  Left (T &&value) noexcept(std::is_nothrow_move_constructible<T>::value)
    : _value(std::move(value))
  {}
};

/**
 * Creates a Left value which can be implicitly converted into an Either.
 *
 * This function allows the Right type to be inferred from context.
 */
template<typename T>
auto makeLeft (T &&value)
{
  return Left<typename std::decay<T>::type>(std::forward<T>(value));
}

/**
 * Temporarily holds the right side of an Either, with an unbound Left type. 
 */
template<typename T>
struct Right final
{
  T _value;

  Right () = delete;

  Right (const T &value) noexcept(std::is_nothrow_copy_constructible<T>::value)
    : _value(value)
  {}

  Right (T &&value) noexcept(std::is_nothrow_move_constructible<T>::value)
    : _value(std::move(value))
  {}
};

/**
 * Creates a Right value which can be implicitly converted into an Either.
 *
 * This function allows the Left type to be inferred from context.
 */
template<typename T>
auto makeRight (T &&value)
{
  return Right<typename std::decay<T>::type>(std::forward<T>(value));
}

/**
 * Represents one of two values.
 *
 * By convention, when used for communicating errors, Right is the successful
 * value (mnemonic "worked right"), while Left contains the error information.
 */
template<typename Left, typename Right>
struct Either final
{
  public:
    /**
     * Type tag used to disambiguate Left values, when the two types are the
     * same.
     */
    struct LeftTag final {};

    /**
     * Type tag used to disambiguate Right values, when the two types are the
     * same.
     */
    struct RightTag final {};

    Either () = delete;

    Either (const Either &other) noexcept(noexcept(copyFrom(other)))
    {
      copyFrom(other);
    }

    Either (Either &&other) noexcept(noexcept(moveFrom(std::move(other))))
    {
      moveFrom(std::move(other));
    }

    Either (Left left, LeftTag tag = LeftTag()) noexcept(std::is_nothrow_move_constructible<Left>::value)
      : _isLeft(true)
      , _value(tag, std::move(left))
    {}

    Either (Right right, RightTag tag = RightTag()) noexcept(std::is_nothrow_move_constructible<Right>::value)
      : _isLeft(false)
      , _value(tag, std::move(right))
    {}

    Either (Arbiter::Left<Left> left) noexcept(std::is_nothrow_move_constructible<Left>::value)
      : Either(std::move(left._value), LeftTag())
    {}

    Either (Arbiter::Right<Right> right) noexcept(std::is_nothrow_move_constructible<Right>::value)
      : Either(std::move(right._value), RightTag())
    {}

    Either &operator= (const Either &other) noexcept(noexcept(destroy()) && noexcept(copyFrom(other)))
    {
      if (this == &other) {
        return *this;
      }

      destroy();
      copyFrom(other);
      return *this;
    }

    Either &operator= (Either &&other) noexcept(noexcept(destroy()) && noexcept(moveFrom(std::move(other))))
    {
      destroy();
      moveFrom(std::move(other));
      return *this;
    }

    ~Either () noexcept(noexcept(destroy()))
    {
      destroy();
    }

    bool hasLeft () const noexcept
    {
      return _isLeft;
    }

    Left &left () noexcept
    {
      assert(hasLeft());
      return _value._left;
    }

    const Left &left () const noexcept
    {
      assert(hasLeft());
      return _value._left;
    }

    bool hasRight () const noexcept
    {
      return !hasLeft();
    }

    Right &right () noexcept
    {
      assert(hasRight());
      return _value._right;
    }

    const Right &right () const noexcept
    {
      assert(hasRight());
      return _value._right;
    }

    /**
     * Returns the Right value, if hasRight() is true, or else throws an
     * exception using the Left value.
     */
    Right &rightOrThrowLeft ()
    {
      if (hasLeft()) {
        throw left();
      } else {
        return right();
      }
    }

    /**
     * Returns the Right value, if hasRight() is true, or else throws an
     * exception using the Left value.
     */
    const Right &rightOrThrowLeft () const
    {
      if (hasLeft()) {
        throw left();
      } else {
        return right();
      }
    }
  
  private:
    union Value
    {
      Left _left;
      Right _right;

      Value ()
      {}

      Value (LeftTag, Left left) noexcept(std::is_nothrow_move_constructible<Left>::value)
        : _left(std::move(left))
      {}

      Value (RightTag, Right right) noexcept(std::is_nothrow_move_constructible<Right>::value)
        : _right(std::move(right))
      {}

      ~Value ()
      {}
    };

    bool _isLeft;
    Value _value;

    void destroy () noexcept(std::is_nothrow_destructible<Left>::value && std::is_nothrow_destructible<Right>::value)
    {
      if (_isLeft) {
        _value._left.~Left();
      } else {
        _value._right.~Right();
      }
    }

    void copyFrom (const Either &other) noexcept(std::is_nothrow_copy_constructible<Left>::value && std::is_nothrow_copy_constructible<Right>::value)
    {
      if ((_isLeft = other._isLeft)) {
        _value._left = other._value._left;
      } else {
        _value._right = other._value._right;
      }
    }

    void moveFrom (Either &&other) noexcept(std::is_nothrow_move_constructible<Left>::value && std::is_nothrow_move_constructible<Right>::value)
    {
      if ((_isLeft = other._isLeft)) {
        _value._left = std::move(other._value._left);
      } else {
        _value._right = std::move(other._value._right);
      }
    }
};

template<typename Left, typename Right>
bool operator== (const Either<Left, Right> &lhs, const Either<Left, Right> &rhs)
{
  if (lhs.hasLeft()) {
    if (rhs.hasLeft()) {
      return lhs.left() == rhs.left();
    } else {
      return false;
    }
  } else if (!rhs.hasLeft()) {
    return lhs.right() == rhs.right();
  } else {
    return false;
  }
}

} // namespace Arbiter

namespace std {

template<typename Left, typename Right>
struct hash<Arbiter::Either<Left, Right>> final
{
  public:
    size_t operator() (const Arbiter::Either<Left, Right> &either) const
    {
      if (either.hasLeft()) {
        return Arbiter::hashOf(either.left());
      } else {
        return Arbiter::hashOf(either.right());
      }
    }
};

} // namespace std
