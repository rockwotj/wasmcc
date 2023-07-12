#pragma once

#include <iostream>
#include <source_location>

#include "absl/strings/str_format.h"

namespace wasmcc {

namespace detail {
struct AssertCondition {
  // NOLINTNEXTLINE(*-explicit-conversions)
  AssertCondition(
      bool c, const std::source_location s = std::source_location::current())
      : value(c), src(s) {}
  bool value;
  std::source_location src;
};
}  // namespace detail

template <typename... Args>
[[noreturn]] void Abort(const std::source_location src,
                        const absl::FormatSpec<Args...>& fmt, Args... args) {
  std::cerr << src.file_name() << ":" << src.line()
            << "Assertion failed: " << absl::StrFormat(fmt, args...)
            << std::endl;
  __builtin_trap();
}

template <typename... Args>
void Assert(detail::AssertCondition cond, const absl::FormatSpec<Args...>& fmt,
            Args... args) {
  if (!cond.value) [[unlikely]] {
    Abort(cond.src, fmt, args...);
  }
}

}  // namespace wasmcc
