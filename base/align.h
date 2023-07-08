#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace wasmcc {

/**
 * Align value down to the nearest 'align' bytes.
 */
template <typename T>
T AlignDown(T v, T alignment) {
  static_assert(std::is_unsigned_v<T>, "alignment requires unsigned types");
  return v & ~(alignment - 1);
}
/**
 * Align value up to the nearest 'align' bytes.
 */
template <typename T>
T AlignUp(T v, T alignment) {
  static_assert(std::is_unsigned_v<T>, "alignment requires unsigned types");
  return AlignDown<T>(v + alignment - 1, alignment);
}

}  // namespace wasmcc
