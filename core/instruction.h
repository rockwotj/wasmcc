#include <array>
#include <variant>
#include <vector>

#include "value.h"

#pragma once

namespace wasmcc {

/**
 * Blocks have types and push/pop stuff from the stack.
 */
struct BlockType {
  std::vector<ValType> parameter_types;
  std::vector<ValType> result_types;

  friend bool operator==(const BlockType&, const BlockType&) = default;
};

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
struct LabelBlockStart {
  explicit LabelBlockStart(BlockType bt) : block_type(std::move(bt)) {}
  BlockType block_type;
};
// An alternative (i.e. else statement) branch
struct LabelBlockAlternative {};
// The end of a block that has a label
struct LabelBlockEnd {};
}  // namespace op

using Instruction =
    std::variant<op::ConstI32, op::AddI32, op::GetLocalI32, op::SetLocalI32,
                 op::Return, op::LabelBlockStart, op::LabelBlockAlternative,
                 op::LabelBlockEnd>;

}  // namespace wasmcc
