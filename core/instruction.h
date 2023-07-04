#include <array>
#include <variant>

#include "value.h"

#pragma once

namespace wasmcc {
namespace op {
// Push the constant onto the top of the stack.
struct ConstI32 {
  explicit ConstI32(uint32_t v) : value(Value::U32(v)) {}
  explicit ConstI32(int32_t v) : value(Value::I32(v)) {}
  explicit ConstI32(Value v) : value(v) {}
  Value value;
};
struct AddI32 {};
// Push the local indexed by `idx` onto the top of the stack.
struct GetLocalI32 {
  explicit GetLocalI32(size_t i) : idx(i) {}
  size_t idx;
};
// Pop the top of the stack into local indexed by `idx`.
struct SetLocalI32 {
  explicit SetLocalI32(size_t i) : idx(i) {}
  size_t idx;
};
// Return the rest of the stack to the caller.
struct Return {};
}  // namespace op

using Instruction = std::variant<op::ConstI32, op::AddI32, op::GetLocalI32,
                                 op::SetLocalI32, op::Return>;

}  // namespace wasmcc
