#pragma once
#include <cstdint>
#include <ostream>

namespace wasmcc {

/**
 * See: https://webassembly.github.io/spec/core/syntax/types.html
 */
enum class ValType : uint8_t {
  kI32 = 0x7F,
  kI64 = 0x7E,
  kF32 = 0x7D,
  kF64 = 0x7C,
  kV128 = 0x7B,
  kFuncRef = 0x70,
  kExternRef = 0x6F,
};
bool is_32bit(ValType);
bool is_64bit(ValType);

std::ostream& operator<<(std::ostream&, ValType);

/**
 * See: https://webassembly.github.io/spec/core/syntax/types.html
 */
union Value {
  uint32_t i32;
  uint64_t i64;
  float f32;
  double f64;
};
std::ostream& operator<<(std::ostream&, ValType);
}  // namespace wasmcc
