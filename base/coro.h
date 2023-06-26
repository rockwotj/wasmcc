#include <coroutine>
#include <exception>
#include <optional>
#include <stdexcept>
#include <utility>
#include <variant>

#pragma once

namespace wasmcc::co {

/**
 * When not building within seastar, this is as simple as it gets future
 * implementation.
 */
template <typename T = void>
class future;

/**
 * Give up the CPU if the scheduler deems it so.
 */
future<> maybe_yield();

namespace detail {

struct awaiter_node {
  std::coroutine_handle<> parent;
  bool await_ready() const noexcept { return !parent; }
  std::coroutine_handle<> await_suspend(
      std::coroutine_handle<>) const noexcept {
    return parent;
  }
  constexpr void await_resume() const noexcept {}
};

template <class T>
class promise final {
 public:
  void return_value(T&& value) noexcept {
    _result.template emplace<1>(std::move(value));
  }
  void return_value(const T& value) noexcept { _result.emplace<1>(value); }
  void unhandled_exception() noexcept {
    _result.template emplace<2>(std::current_exception());
  }
  future<T> get_return_object() noexcept { return future<T>(this); }
  constexpr std::suspend_always initial_suspend() const noexcept { return {}; }
  awaiter_node final_suspend() const noexcept { return _chain; }

  friend class future<T>;

 private:
  awaiter_node _chain;
  std::variant<std::monostate, T, std::exception_ptr> _result;
};

}  // namespace detail

template <typename T>
class [[nodiscard("must not drop futures")]] future {
 public:
  // coroutine interface
  using promise_type = detail::promise<T>;
  explicit future(promise_type * p) noexcept
      : _handle(std::coroutine_handle<promise_type>::from_promise(*p)) {}

  T get() {
    auto handle = await_suspend(std::noop_coroutine());
    while (!handle.done()) {
      handle.resume();
    }
    return await_resume();
  }

  // awaitable interface
  constexpr bool await_ready() const noexcept { return false; }
  std::coroutine_handle<> await_suspend(std::coroutine_handle<> parent)
      const noexcept {
    _handle.promise()._chain.parent = parent;
    return _handle;
  }
  decltype(auto) await_resume() {
    if (_handle.promise()._result.index() == 1) {
      return std::move(std::get<1>(_handle.promise()._result));
    } else {
      std::rethrow_exception(
          std::get<std::exception_ptr>(_handle.promise()._result));
    }
  };

  // resource management
  future(future&) = delete;
  future& operator=(future&) = delete;
  future(future && moved) noexcept
      : _handle(std::exchange(moved._handle, nullptr)) {}
  future& operator=(future&& moved) noexcept {
    _handle = std::exchange(moved._handle, nullptr);
  }
  explicit operator std::coroutine_handle<>() {
    return std::exchange(_handle, nullptr);
  }
  ~future() noexcept {
    if (_handle) {
      _handle.destroy();
    }
  }

 private:
  std::coroutine_handle<promise_type> _handle;
};

namespace detail {
template <>
class promise<void> final {
 public:
  constexpr void return_void() const noexcept {}
  void unhandled_exception() noexcept { _result = std::current_exception(); }
  future<> get_return_object() noexcept { return future<>(this); }
  constexpr std::suspend_always initial_suspend() const noexcept { return {}; }
  awaiter_node final_suspend() const noexcept { return _chain; }

 private:
  friend class future<>;

  awaiter_node _chain;
  std::optional<std::exception_ptr> _result;
};
}  // namespace detail

template <>
inline decltype(auto) future<void>::await_resume() {
  const auto& result = _handle.promise()._result;
  if (result.has_value()) {
    std::rethrow_exception(result.value());
  }
}

}  // namespace wasmcc::co
