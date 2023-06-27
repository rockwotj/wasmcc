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
class Future;

/**
 * Give up the CPU if the scheduler deems it so.
 */
Future<> MaybeYield();

namespace detail {

struct AwaiterNode {
  std::coroutine_handle<> parent;
  bool await_ready() const noexcept { return !parent; }
  std::coroutine_handle<> await_suspend(
      std::coroutine_handle<>) const noexcept {
    return parent;
  }
  constexpr void await_resume() const noexcept {}
};

template <class T>
class Promise final {
 public:
  void return_value(T&& value) noexcept {
    _result.template emplace<1>(std::move(value));
  }
  void return_value(const T& value) noexcept { _result.emplace<1>(value); }
  void unhandled_exception() noexcept {
    _result.template emplace<2>(std::current_exception());
  }
  Future<T> get_return_object() noexcept { return Future<T>(this); }
  constexpr std::suspend_always initial_suspend() const noexcept { return {}; }
  AwaiterNode final_suspend() const noexcept { return _chain; }

  friend class Future<T>;

 private:
  AwaiterNode _chain;
  std::variant<std::monostate, T, std::exception_ptr> _result;
};

}  // namespace detail

template <typename T>
class [[nodiscard("must not drop futures")]] Future {
 public:
  // coroutine interface
  using promise_type = detail::Promise<T>;
  explicit Future(promise_type* p) noexcept
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
  std::coroutine_handle<> await_suspend(
      std::coroutine_handle<> parent) const noexcept {
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
  Future(Future&) = delete;
  Future& operator=(Future&) = delete;
  Future(Future&& moved) noexcept
      : _handle(std::exchange(moved._handle, nullptr)) {}
  Future& operator=(Future&& moved) noexcept {
    _handle = std::exchange(moved._handle, nullptr);
  }
  explicit operator std::coroutine_handle<>() {
    return std::exchange(_handle, nullptr);
  }
  ~Future() noexcept {
    if (_handle) {
      _handle.destroy();
    }
  }

 private:
  std::coroutine_handle<promise_type> _handle;
};

namespace detail {
template <>
class Promise<void> final {
 public:
  constexpr void return_void() const noexcept {}
  void unhandled_exception() noexcept { _result = std::current_exception(); }
  Future<> get_return_object() noexcept { return Future<>(this); }
  constexpr std::suspend_always initial_suspend() const noexcept { return {}; }
  AwaiterNode final_suspend() const noexcept { return _chain; }

 private:
  friend class Future<>;

  AwaiterNode _chain;
  std::optional<std::exception_ptr> _result;
};
}  // namespace detail

template <>
inline decltype(auto) Future<void>::await_resume() {
  const auto& result = _handle.promise()._result;
  if (result.has_value()) {
    std::rethrow_exception(result.value());
  }
}

}  // namespace wasmcc::co
