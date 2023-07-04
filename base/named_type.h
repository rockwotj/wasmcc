#pragma once

#include <algorithm>
#include <string_view>

namespace wasmcc {

/**
 * A strongly typed wrapper for primative types like ints and strings.
 *
 * Usage:
 *
 * using MyId = NamedType<int, struct MyIdTag>;
 */
template <typename T, typename Tag>
class NamedType {
 public:
  explicit NamedType(T value) : _value(std::move(value)) {}
  NamedType(const NamedType&) = default;
  NamedType& operator=(const NamedType&) = default;
  NamedType(NamedType&&) noexcept = default;
  NamedType& operator=(NamedType&&) noexcept = default;
  ~NamedType() = default;

  T take() && { return std::move(_value); }
  T value() const { return _value; }

  friend bool operator<=>(NamedType<T, Tag>, NamedType<T, Tag>) = default;

  template <typename H>
  friend H AbslHashValue(H h, const NamedType<T, Tag>& t) {
    return H::combine(std::move(h), t._value);
  }

 private:
  T _value;
};

}  // namespace wasmcc
