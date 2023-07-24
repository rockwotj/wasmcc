#include <array>
#include <string>
#include <variant>
#include <vector>

#include "base/named_type.h"
#include "value.h"

#pragma once

namespace wasmcc {

using LabelId = NamedType<std::string, struct LabelIdTag>;

// Make a label in the form of <str>_<id>
//
// Most labels have a "class" so to speak and then an numeric id to prevent
// clashes, so this simplifies creating those.
LabelId MakeLabelId(std::string_view, int32_t);

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

// The start of a block
struct Label {
  LabelId id;
};
// Branch to a given label ID
struct Br {
  LabelId label_id;
};
// Branch to `then_label_id` if the top of the stack is non zero, otherwise
// branch to `else_label_id`.
struct BrIf {
  LabelId then_label_id;
  LabelId else_label_id;
};
}  // namespace op

using Instruction =
    std::variant<op::ConstI32, op::AddI32, op::GetLocalI32, op::SetLocalI32,
                 op::Return, op::Label, op::Br, op::BrIf>;

}  // namespace wasmcc
