#include "core/value.h"

#include <ios>

namespace wasmcc {
bool is_32bit(ValType vt) {
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
bool is_64bit(ValType vt) {
  switch (vt) {
    case ValType::kI64:
    case ValType::kF64:
      return true;
    default:
      return false;
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
  return os << "0x" << std::hex << std::uppercase << v.i64 << std::nouppercase
            << std::dec;
}
}  // namespace wasmcc
