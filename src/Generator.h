#pragma once

#ifndef __cplusplus
#error "This file must be compiled as C++."
#endif

#include "Either.h"
#include "Future.h"
#include "Optional.h"

#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace Arbiter {

/**
 * A generator function which will (possibly asynchronously) yield values of
 * T until exhausted or an error occurs.
 */
template<typename T>
class Generator final
{
  public:
    using EventFuture = Future<Optional<T>>;
    using Function = std::function<EventFuture(void)>;

    /**
     * Instantiates an exhausted generator.
     */
    Generator ()
      : _fn(nullptr)
    {}

    /**
     * Instantiates a generator with the given callable object.
     *
     * Once the function returns an empty future, or a future containing an
     * error, it will be destroyed.
     */
    explicit Generator (Function fn)
      : _fn(std::move(fn))
    {}

    Generator (const Generator &) = delete;

    Generator (Generator &&other)
      : _fn(std::move(other._fn))
    {}

    Generator &operator= (const Generator &) = delete;

    Generator &operator= (Generator &&other)
    {
      std::lock_guard<std::mutex> lock(_mutex);

      _fn = std::move(other._fn);

      return *this;
    }

    /**
     * Begins generating the next value.
     *
     * This method must not be invoked if empty() is true.
     *
     * Returns a future which will eventually contain the value, an error if one
     * occurred, or an empty Optional if the generator is now exhausted. The
     * generator must not be destroyed until the future is satisfied.
     */
    EventFuture next ()
    {
      std::unique_lock<std::mutex> lock(_mutex);

      Function fn = _fn;
      assert(fn);

      lock.unlock();

      EventFuture future = fn();
      future.add_callback([this] (const typename EventFuture::Result &result) {
        handleResult(result);
      });

      return future;
    }

    /**
     * Returns whether the generator is empty (exhausted).
     */
    bool empty () const
    {
      std::lock_guard<std::mutex> lock(_mutex);
      return !static_cast<bool>(_fn);
    }

  private:
    std::mutex _mutex;
    Function _fn;

    void handleResult (const typename EventFuture::Result &result)
    {
      if (result.hasRight() && result.right()) {
        // Just a regular value, no terminal event yet.
        return;
      }

      std::lock_guard<std::mutex> lock(_mutex);

      assert(_fn);
      _fn = nullptr;
    }
};

/**
 * Implements an optional "write" end to a Generator, similar to how a Promise
 * is the write end of a Future.
 */
template<typename T>
class Sink final
{
  public:
    using Event = Either<std::exception_ptr, Optional<T>>;

    Sink ()
      : _state(std::make_shared<State>())
    {}

    Sink (const Sink &) = delete;

    Sink (Sink &&other)
      : _state(std::move(other._state))
    {}

    Sink &operator= (const Sink &) = delete;

    Sink &operator= (Sink &&other)
    {
      _state = std::move(other._state);
      return *this;
    }

    /**
     * Enqueues an event for the Generator.
     */
    void onEvent (Event event)
    {
      {
        std::lock_guard<std::mutex> lock(_state->_mutex);
        _state->_eventQueue.push(std::move(event));
      }

      _state->_queuePushed.notify_all();
    }

    /**
     * Enqueues a value for the Generator.
     */
    void onNext (T value)
    {
      onEvent(Event(makeOptional(std::move(value))));
    }

    /**
     * Enqueues an error for the Generator.
     *
     * Once this method has been invoked, no more events must be sent to the
     * Sink. 
     */
    void onError (std::exception_ptr ex)
    {
      onEvent(Event(std::move(ex)));
    }

    /**
     * Enqueues a completion event for the Generator.
     *
     * Once this method has been invoked, no more events must be sent to the
     * Sink. 
     */
    void onCompleted ()
    {
      onEvent(Event(Optional<T>()));
    }

    /**
     * Creates a generator for accessing the events which are or will be enqueued upon
     * this sink.
     *
     * Using more than one Generator from the same Sink is undefined behavior.
     */
    Generator<T> getGenerator ()
    {
      return Generator<T>([state = _state] {
        Promise<Optional<T>> promise;
        auto future = promise.get_future();

        std::unique_lock<std::mutex> lock(state->_mutex);
        if (state->_eventQueue.empty()) {
          lock.unlock();

          // TODO: Implement this algorithm without spawning a thread.
          std::thread([state, promise = std::move(promise)]() mutable {
            std::unique_lock<std::mutex> lock(state->_mutex);

            state->_queuePushed.wait(lock, [&state] {
              return !state->_eventQueue.empty();
            });

            popEventAndSetPromise(state->_eventQueue, promise, lock);
          }).detach();
        } else {
          popEventAndSetPromise(state->_eventQueue, promise, lock);
        }

        return future;
      });
    }

  private:
    struct State final
    {
      public:
        // All below fields must only be used while synchronized on this mutex.
        std::mutex _mutex;

        std::queue<Event> _eventQueue;
        std::condition_variable _queuePushed;
    };

    std::shared_ptr<State> _state;

    static void popEventAndSetPromise (std::queue<Event> &queue, Promise<Optional<T>> &promise, std::unique_lock<std::mutex> &lock)
    {
      Event event = queue.front();
      queue.pop();
      lock.unlock();

      promise.set_result(std::move(event));
    }
};

} // namespace Arbiter
