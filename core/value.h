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
bool IsValType32Bit(ValType);
bool IsValType64Bit(ValType);
size_t ValTypeSizeBytes(ValType);

std::ostream& operator<<(std::ostream&, ValType);

/**
 * See: https://webassembly.github.io/spec/core/syntax/types.html
 */
class Value {
 public:
  Value() = default;
  static Value I32(int32_t);
  static Value U32(uint32_t);
  static Value I64(int64_t);
  static Value U64(uint64_t);
  static Value F32(float);
  static Value F64(double);

  int32_t AsI32() const;
  uint32_t AsU32() const;
  int64_t AsI64() const;
  uint64_t AsU64() const;
  float AsF32() const;
  double AsF64() const;

  bool operator==(const Value&) const = default;

 private:
  explicit Value(uint64_t);
  uint64_t _underlying;
};
std::ostream& operator<<(std::ostream&, ValType);
}  // namespace wasmcc
