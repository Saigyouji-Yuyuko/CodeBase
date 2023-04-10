#pragma once
#include <boost/stacktrace.hpp>
#include <coroutine>
#include <cstdint>
#include <exception>
#include <functional>
#include <iostream>
#include <limits>
#include <list>
#include <memory>
#include <thread>
#include <type_traits>

namespace CodeBase {
template<typename T>
class TaskPromise;

template<typename T>
class Task;

template<typename T>
class TaskAwaiter {
public:
    friend Task<T>;
    bool await_ready() {
        return !m_handle || m_handle.done();
    }
    std::coroutine_handle<> await_suspend(std::coroutine_handle<> h);
    decltype(auto)          await_resume();

private:
    TaskAwaiter(std::coroutine_handle<TaskPromise<T>> handle) : m_handle(handle) {}
    std::coroutine_handle<TaskPromise<T>> m_handle;
};

template<typename T = void>
class Task {
public:
    using promise_type = TaskPromise<T>;
    friend promise_type;

    ~Task() {
        if (m_handle && m_handle.done()) {

            m_handle.destroy();
        }
    }

    Task(Task &&other) : m_handle(other.m_handle) {
        other.m_handle = nullptr;
    }
    Task(const Task &) = delete;

    Task &operator=(Task &&other) {
        if (std::addressof(other) != this) {
            std::swap(m_handle, other.m_handle);
            if (other.m_handle) {
                other.m_handle.destroy();
            }
        }
        return *this;
    }
    Task &operator=(const Task &) = delete;

    TaskAwaiter<T> operator co_await() const noexcept {
        return TaskAwaiter<T>{m_handle};
    }

    bool is_ready() const noexcept {
        return !m_handle || m_handle.done();
    }

    decltype(auto) sync() {
        if (!is_ready()) {
            m_handle.resume();
        }
        return m_handle.promise().result();
    }

private:
    Task(std::coroutine_handle<promise_type> handle) : m_handle(handle) {}
    std::coroutine_handle<promise_type> m_handle;
};
struct final_awaitable {
    bool await_ready() const noexcept {
        return false;
    }

    template<typename PROMISE>
    std::coroutine_handle<> await_suspend(std::coroutine_handle<PROMISE> coro) noexcept {
        return coro.promise().m_continuation == nullptr ? std::noop_coroutine() : coro.promise().m_continuation;
    }

    void await_resume() noexcept {}
};
template<typename T>
class TaskPromiseBase {

public:
    friend final_awaitable;
    friend TaskAwaiter<T>;
    std::suspend_always initial_suspend() {
        return {};
    }

    final_awaitable final_suspend() noexcept {

        return final_awaitable{};
    }

private:
    std::coroutine_handle<> m_continuation = nullptr;
};

template<typename T>
class TaskPromise : public TaskPromiseBase<T> {
public:
    TaskPromise() = default;

    Task<T> get_return_object() {
        return Task<T>{std::coroutine_handle<TaskPromise<T>>::from_promise(*this)};
    }

    void unhandled_exception() {
        ::new (static_cast<void *>(std::addressof(m_exception))) std::exception_ptr(std::current_exception());
        m_state = State::Exception;
    }

    ~TaskPromise() {
        if (m_state == State::Value) {
            m_value.~T();
        } else if (m_state == State::Exception) {
            m_exception.~exception_ptr();
        }
    }

    T result() {
        if (m_state == State::Value) {
            return std::move(m_value);
        } else if (m_state == State::Exception) {
            std::rethrow_exception(m_exception);
        } else {

            throw std::logic_error("Task not ready");
        }
    }

    template<typename VALUE, typename = std::enable_if_t<std::is_convertible_v<VALUE &&, T>>>
    void return_value(VALUE &&value) noexcept(std::is_nothrow_constructible_v<T, VALUE &&>) {

        ::new (static_cast<void *>(std::addressof(m_value))) T(std::forward<VALUE>(value));
        m_state = State::Value;
    }

private:
    enum class State {
        Empty,
        Value,
        Exception,
    };
    State m_state = State::Empty;

    union {
        T                  m_value;
        std::exception_ptr m_exception = nullptr;
    };
};

template<>
class TaskPromise<void> : public TaskPromiseBase<void> {
public:
    Task<> get_return_object() {
        return Task<>{std::coroutine_handle<TaskPromise<void>>::from_promise(*this)};
    }
    void return_void() {}

    void unhandled_exception() {
        m_exception = std::current_exception();
    }

    ~TaskPromise() = default;

    void result() {
        if (m_exception)
            std::rethrow_exception(m_exception);
    }

private:
    std::exception_ptr m_exception;
};


template<typename T>
std::coroutine_handle<> TaskAwaiter<T>::await_suspend(std::coroutine_handle<> h) {
    m_handle.promise().m_continuation = h;


    return m_handle;
}

template<typename T>
decltype(auto) TaskAwaiter<T>::await_resume() {

    if (!m_handle.done())
        throw std::runtime_error("Task not done");

    return m_handle.promise().result();
}

}// namespace CodeBase