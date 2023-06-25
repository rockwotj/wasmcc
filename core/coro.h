#include <coroutine>
#include <exception>
#include <optional>
#include <stdexcept>
#include <utility>
#include <variant>

#pragma once

namespace wasmcc {

/**
 * When not building within seastar, this is as simple as it gets future
 * implementation.
 */
template<typename T = void>
class future;

namespace detail {

struct awaiter_node {
    std::coroutine_handle<> parent;
    bool await_ready() const noexcept { return !parent; }
    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<>) const noexcept {
        return parent;
    }
    constexpr void await_resume() const noexcept {}
};

template<class T>
class chain_promise final {
public:
    void return_value(T&& value) noexcept {
        _result.template emplace<1>(std::move(value));
    }
    void return_value(const T& value) noexcept { _result.emplace<1>(value); }
    void unhandled_exception() noexcept {
        _result.template emplace<2>(std::current_exception());
    }
    future<T> get_return_object() noexcept { return future<T>(this); }
    constexpr std::suspend_always initial_suspend() const noexcept {
        return {};
    }
    awaiter_node final_suspend() const noexcept { return _chain; }
    awaiter_node* awaiter() { return &_chain; }

    friend class future<T>;

private:
    awaiter_node _chain;
    std::variant<std::monostate, T, std::exception_ptr> _result;
};

} // namespace detail

template<typename T>
class [[nodiscard("must not drop futures")]] future {
public:
    // coroutine interface
    using promise_type = detail::chain_promise<T>;
    explicit future(promise_type* p) noexcept
      : handle(std::coroutine_handle<promise_type>::from_promise(*p)) {}

    T get() {
        auto handle = await_suspend(std::noop_coroutine());
        while (!handle.done()) {
            handle.resume();
        }
        return await_resume();
    }

    // awaitable interface
    constexpr bool await_ready() const noexcept { return false; }
    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<> parent) const noexcept {
        handle.promise().awaiter()->parent = parent;
        return handle;
    }
    decltype(auto) await_resume() {
        if (handle.promise()._result.index() == 1) {
            return std::move(std::get<1>(handle.promise()._result));
        } else {
            std::rethrow_exception(
              std::get<std::exception_ptr>(handle.promise()._result));
        }
    };

    // resource management
    future(future&) = delete;
    future& operator=(future&) = delete;
    future(future&& moved) noexcept
      : handle(moved.handle) {
        moved.handle = nullptr;
    }
    future& operator=(future&& moved) noexcept {
        handle = std::exchange(moved.handle, nullptr);
    }
    explicit operator std::coroutine_handle<>() {
        auto tmp = handle;
        handle = nullptr;
        return tmp;
    }
    ~future() noexcept {
        if (handle) {
            handle.destroy();
        }
    }

private:
    std::coroutine_handle<promise_type> handle;
};

namespace detail {
template<>
class chain_promise<void> final {
    constexpr void return_void() const noexcept {}
    void unhandled_exception() noexcept { _result = std::current_exception(); }
    future<> get_return_object() noexcept { return future<>(this); }
    constexpr std::suspend_always initial_suspend() const noexcept {
        return {};
    }
    awaiter_node final_suspend() const noexcept { return _chain; }
    awaiter_node* awaiter() { return &_chain; }

private:
    friend class future<>;

    awaiter_node _chain;
    std::optional<std::exception_ptr> _result;
};
} // namespace detail

template<>
inline decltype(auto) future<void>::await_resume() {
    const auto& result = handle.promise()._result;
    if (result.has_value()) {
        std::rethrow_exception(result.value());
    }
}

} // namespace wasmcc
