#include "core/value.h"

#include <bit>
#include <cstdint>
#include <exception>
#include <ios>

namespace wasmcc {
bool IsValType32Bit(ValType vt) {
  switch (vt) {
    case ValType::kI32:
    case ValType::kF32:
    case ValType::kExternRef:
    case ValType::kFuncRef:
      return true;
    default:
      return false;
  }
}
bool IsValType64Bit(ValType vt) {
  switch (vt) {
    case ValType::kI64:
    case ValType::kF64:
      return true;
    default:
      return false;
  }
}
size_t ValTypeSizeBytes(ValType vt) {
  switch (vt) {
    case ValType::kI32:
      return sizeof(int32_t);
    case ValType::kI64:
      return sizeof(int64_t);
    case ValType::kF32:
      return sizeof(float);
    case ValType::kF64:
      return sizeof(double);
    case ValType::kV128:
      return sizeof(int64_t) * 2;
    case ValType::kFuncRef:
    case ValType::kExternRef:
      return sizeof(intptr_t);
    default:
      __builtin_unreachable();
  }
}
std::ostream& operator<<(std::ostream& os, ValType vt) {
  switch (vt) {
    case ValType::kI32:
      return os << "i32";
    case ValType::kI64:
      return os << "i64";
    case ValType::kF32:
      return os << "f32";
    case ValType::kF64:
      return os << "f64";
    case ValType::kV128:
      return os << "v128";
    case ValType::kFuncRef:
      return os << "funcref";
    case ValType::kExternRef:
      return os << "externref";
  }
  return os << "unknown";
}
std::ostream& operator<<(std::ostream& os, Value v) {
  return os << "0x" << std::hex << std::uppercase << v.AsI64()
            << std::nouppercase << std::dec;
}

Value::Value(uint64_t v) : _underlying(v) {}
Value Value::I32(int32_t v) { return Value(uint64_t(v)); }
Value Value::U32(uint32_t v) { return Value(uint64_t(v)); }
Value Value::I64(int64_t v) { return Value(v); }
Value Value::U64(uint64_t v) { return Value(v); }
Value Value::F32(float v) { return Value(std::bit_cast<uint32_t>(v)); }
Value Value::F64(double v) { return Value(std::bit_cast<uint64_t>(v)); }

int32_t Value::AsI32() const { return int32_t(_underlying); }
uint32_t Value::AsU32() const { return uint32_t(_underlying); }
int64_t Value::AsI64() const { return int64_t(_underlying); }
uint64_t Value::AsU64() const { return _underlying; }
float Value::AsF32() const {
  return std::bit_cast<float>(uint32_t(_underlying));
}
double Value::AsF64() const { return std::bit_cast<double>(_underlying); }
}  // namespace wasmcc
