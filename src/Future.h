#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include "Either.h"
#include "Optional.h"

#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>

namespace Arbiter {

/**
 * Future type resembling std::shared_future, with support for asynchronous
 * callbacks and composition.
 */
template<typename T>
class Future;

/**
 * Promise type resembling std::promise, which yields Arbiter::Future instead
 * of std::future or std::shared_future.
 */
template<typename T>
class Promise
{
  public:
    Promise ()
      : _state(std::make_shared<State>())
    {}

    Promise (const Promise &) = delete;

    Promise (Promise &&other)
      : _state(std::move(other._state))
    {}

    Promise &operator= (const Promise &) = delete;

    Promise &operator= (Promise &&other)
    {
      _state = std::move(other._state);
      return *this;
    }

    Future<T> get_future () const
    {
      return Future<T>(_state);
    }

    void set_value (const T &value)
    {
      set_storage(makeRight(value));
    }

    void set_value (T &&value)
    {
      set_storage(makeRight(std::move(value)));
    }

    void set_exception (std::exception_ptr ex)
    {
      set_storage(makeLeft(ex));
    }

  private:
    friend class Future<T>;

    struct State
    {
      // All below fields must only be used while synchronized on this mutex.
      std::mutex _mutex;

      Optional<Either<std::exception_ptr, T>> _storage;
      std::condition_variable _satisfied;
    };

    std::shared_ptr<State> _state;

    void set_storage (Either<std::exception_ptr, T> &&storage)
    {
      {
        std::lock_guard<std::mutex> lock(_state->_mutex);
        _state->_storage = makeOptional(std::move(storage));
      }

      _state->_satisfied.notify_all();
    }
};

template<typename T>
class Future
{
  public:
    Future ()
    {}

    Future (const Future &other)
      : _state(other._state)
    {}

    Future (Future &&other)
      : _state(std::move(other._state))
    {}

    Future &operator= (const Future &other)
    {
      _state = other._state;
      return *this;
    }

    Future &operator= (Future &&other)
    {
      _state = std::move(other._state);
      return *this;
    }

    bool valid () const noexcept
    {
      return static_cast<bool>(_state);
    }

    T &get ()
    {
      std::unique_lock<std::mutex> lock(_state->_mutex);
      wait(lock);

      if (_state->_storage->hasLeft()) {
        throw _state->_storage->left();
      } else {
        return _state->_storage->right();
      }
    }

    const T &get () const
    {
      std::unique_lock<std::mutex> lock(_state->_mutex);
      wait(lock);

      if (_state->_storage->hasLeft()) {
        throw _state->_storage->left();
      } else {
        return _state->_storage->right();
      }
    }

    void wait () const
    {
      std::unique_lock<std::mutex> lock(_state->_mutex);
      wait(lock);
    }

  private:
    friend class Promise<T>;
    using State = typename Promise<T>::State;

    Future (const std::shared_ptr<State> &state)
      : _state(state)
    {}

    void wait (std::unique_lock<std::mutex> &lock) const
    {
      _state->_satisfied.wait(lock, [state = _state] {
        return static_cast<bool>(state->_storage);
      });
    }

    std::shared_ptr<State> _state;
};

}
