#include "parser/validator.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "core/value.h"

namespace wasmcc {

ValidationType::ValidationType(ValType vt) : _type(uint8_t(vt)) {}

ValidationType ValidationType::any() {
  ValidationType vt;
  vt._type = 0;
  return vt;
}
ValidationType ValidationType::anyref() {
  ValidationType vt;
  vt._type = 1;
  return vt;
}
bool ValidationType::is_i32() const { return _type == uint8_t(ValType::kI32); }
bool ValidationType::is_any() const { return _type == kAnyTypeValue; }
bool ValidationType::is_anyref() const { return _type == kAnyRefTypeValue; }
bool ValidationType::is_ref() const {
  return _type == kAnyRefTypeValue || _type == uint8_t(ValType::kFuncRef) ||
         _type == uint8_t(ValType::kExternRef);
}
size_t ValidationType::size_bytes() const {
  switch (_type) {
    case kAnyTypeValue:  // Assume worst case for anytype
      return sizeof(uint64_t) * 2;
    case kAnyRefTypeValue:
      return sizeof(intptr_t);
    default:
      return ValTypeSizeBytes(ValType(_type));
  }
}
std::ostream& operator<<(std::ostream& os, ValidationType vt) {
  switch (vt._type) {
    case uint8_t(ValType::kI32):
      return os << "i32";
    case uint8_t(ValType::kI64):
      return os << "i64";
    case uint8_t(ValType::kF32):
      return os << "f32";
    case uint8_t(ValType::kF64):
      return os << "f64";
    case ValidationType::kAnyTypeValue:
      return os << "{any}";
    case uint8_t(ValType::kV128):
      return os << "v128";
    case ValidationType::kAnyRefTypeValue:
      return os << "{anyref}";
    case uint8_t(ValType::kFuncRef):
      return os << "funcref";
    case uint8_t(ValType::kExternRef):
      return os << "externref";
    default:
      __builtin_unreachable();
  }
}
FunctionValidator::FunctionValidator(FunctionSignature ft,
                                     const std::vector<ValType>& locals)
    : _locals(std::move(ft.parameter_types)),
      _returns(std::move(ft.result_types)) {
  std::copy(locals.begin(), locals.end(), std::back_inserter(_locals));
}

size_t FunctionValidator::maximum_stack_elements() const {
  return _max_stack_size;
}
size_t FunctionValidator::maximum_stack_size_bytes() const {
  return _max_memory_usage;
}

void FunctionValidator::operator()(const op::ConstI32&) { Push(ValType::kI32); }
void FunctionValidator::operator()(const op::AddI32&) {
  Pop(ValType::kI32);
  Pop(ValType::kI32);
  Push(ValType::kI32);
}
void FunctionValidator::operator()(const op::GetLocalI32& op) {
  AssertLocal(op.idx, ValType::kI32);
  Push(ValType::kI32);
}
void FunctionValidator::operator()(const op::SetLocalI32& op) {
  Pop(ValType::kI32);
  AssertLocal(op.idx, ValType::kI32);
}
void FunctionValidator::operator()(const op::Return&) {
  for (ValType vt : _returns) {
    Pop(vt);
  }
  AssertEmpty();
  // Mark the current block as unreachable.
  _unreachable = true;
}

void FunctionValidator::Finalize() {
  for (ValType vt : _returns) {
    Pop(vt);
  }
  AssertEmpty();
}
bool FunctionValidator::empty() const { return _underlying.empty(); }

void FunctionValidator::AssertLocal(size_t idx, ValType vt) const {
  if (idx >= _locals.size() || _locals[idx] != vt) [[unlikely]] {
    throw ValidationException();
  }
}
void FunctionValidator::AssertEmpty() const {
  if (!_underlying.empty()) [[unlikely]] {
    throw ValidationException();
  }
}
void FunctionValidator::Pop(ValType vt) { Pop(ValidationType(vt)); }
void FunctionValidator::Pop(ValidationType expected) {
  ValidationType actual = ValidationType::any();
  if (_underlying.empty()) {
    if (!_unreachable) {
      throw ValidationException();
    }
  } else {
    actual = _underlying.back();
    _underlying.pop_back();
  }
  _current_memory_usage -= actual.size_bytes();
  bool ok = actual == expected;
  if (actual.is_any() || expected.is_any()) {
    ok = true;
  } else if (expected.is_anyref()) {
    ok = actual.is_ref();
  }
  if (!ok) {
    throw ValidationException();
  }
}
void FunctionValidator::Push(ValType vt) { Push(ValidationType(vt)); }

void FunctionValidator::Push(ValidationType vt) {
  _underlying.push_back(vt);
  _current_memory_usage += vt.size_bytes();
  _max_stack_size = std::max(_max_stack_size, _underlying.size());
  _max_memory_usage = std::max(_max_memory_usage, _current_memory_usage);
}
}  // namespace wasmcc
