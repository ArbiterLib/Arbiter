#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include "Either.h"
#include "Optional.h"

#include <condition_variable>
#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <vector>

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
class Promise final
{
  public:
    /**
     * The type of result which will be provided by any future obtained from
     * this promise.
     */
    using Result = Either<std::exception_ptr, T>;

    Promise ()
      : _state(std::make_shared<State>())
    {}

    Promise (const Promise &) = delete;

    Promise (Promise &&other)
      : _state(std::move(other._state))
    {}

    ~Promise ()
    {
      if (!_state->_result) {
        throw std::future_error(std::future_errc::broken_promise);
      }
    }

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
      set_result(makeRight(value));
    }

    void set_value (T &&value)
    {
      set_result(makeRight(std::move(value)));
    }

    void set_exception (std::exception_ptr ex)
    {
      set_result(makeLeft(ex));
    }

  private:
    friend class Future<T>;

    using Callback = std::function<void(const Result &)>;

    struct State
    {
      // All below fields must only be used while synchronized on this mutex.
      std::mutex _mutex;

      Optional<Result> _result;
      std::vector<Callback> _callbacks;
      std::condition_variable _satisfied;
    };

    std::shared_ptr<State> _state;

    void set_result (Result &&result)
    {
      std::vector<Callback> callbacks;
      const Result *resultPtr;

      {
        std::lock_guard<std::mutex> lock(_state->_mutex);

        if (_state->_result) {
          throw std::future_error(std::future_errc::promise_already_satisfied);
        }

        _state->_result = makeOptional(std::move(result));

        // Move callbacks so we can invoke them outside of the lock.
        _state->_callbacks.swap(callbacks);
        resultPtr = _state->_result.pointer();
      }

      // Notify synchronous observers first, to unblock them.
      _state->_satisfied.notify_all();

      for (auto &callback : callbacks) {
        // Now that the result has been assigned within the mutex, it should be
        // safe to read it from outside of one.
        callback(*resultPtr);
      }
    }
};

template<typename T>
class Future final
{
  public:
    /**
     * The type of result that will be provided by this future.
     */
    using Result = typename Promise<T>::Result;

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
      enforce_valid();

      std::unique_lock<std::mutex> lock(_state->_mutex);
      wait(lock);

      if (_state->_result->hasLeft()) {
        throw _state->_result->left();
      } else {
        return _state->_result->right();
      }
    }

    const T &get () const
    {
      enforce_valid();

      std::unique_lock<std::mutex> lock(_state->_mutex);
      wait(lock);

      if (_state->_result->hasLeft()) {
        throw _state->_result->left();
      } else {
        return _state->_result->right();
      }
    }

    void wait () const
    {
      enforce_valid();

      std::unique_lock<std::mutex> lock(_state->_mutex);
      wait(lock);
    }

    /**
     * Adds a callback which will be invoked with the Result once the future has
     * been satisfied.
     *
     * If the future is already satisfied, invokes the callback synchronously.
     */
    template<typename Callable>
    void add_callback (Callable callback)
    {
      enforce_valid();

      std::unique_lock<std::mutex> lock(_state->_mutex);
      if (_state->_result) {
        lock.unlock();
        callback(_state->_result.value());
      } else {
        _state->_callbacks.emplace_back(std::move(callback));
      }
    }

  private:
    friend class Promise<T>;
    using State = typename Promise<T>::State;

    Future (const std::shared_ptr<State> &state)
      : _state(state)
    {}

    void enforce_valid () const
    {
      if (!valid()) {
        throw std::future_error(std::future_errc::no_state);
      }
    }

    void wait (std::unique_lock<std::mutex> &lock) const
    {
      _state->_satisfied.wait(lock, [state = _state] {
        return static_cast<bool>(state->_result);
      });
    }

    std::shared_ptr<State> _state;
};

} // namespace Arbiter
