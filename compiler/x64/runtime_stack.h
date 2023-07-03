#pragma once

#include <span>

#include "absl/container/fixed_array.h"
#include "compiler/x64/register_tracker.h"
#include "core/value.h"

namespace wasmcc::x64 {

struct RuntimeValue {
  // The offset relative to the base of the stack in memory where this stack
  // value can spill to if not in a register.
  int32_t stack_pointer = 0;
  // The register that this value is located in.
  std::optional<asmjit::x86::Gp> reg;
  // The type of this register
  ValType type = ValType::kI32;

  bool operator==(const RuntimeValue&) const = default;

  // The size of this value on the stack in bytes.
  size_t size_bytes() const;
};

/**
 * This class tracks what is on the stack during compilation.
 *
 * It also is responsible for tracking stack value's location in stack memory.
 */
class RuntimeStack {
  using UnderlyingType = absl::FixedArray<RuntimeValue>;

 public:
  RuntimeStack(size_t max_stack_elements, RegisterTracker*);
  RuntimeStack(const RuntimeStack&) = delete;
  RuntimeStack& operator=(const RuntimeStack&) = delete;
  RuntimeStack(RuntimeStack&&) = delete;
  RuntimeStack& operator=(RuntimeStack&&) = delete;
  ~RuntimeStack() = default;

  RuntimeValue* Push(RuntimeValue);
  [[nodiscard]] RuntimeValue Pop();
  RuntimeValue* Peek();
  const RuntimeValue* Peek() const;

  std::span<RuntimeValue> ReverseIterator();

  // The current offset in bytes from the bottom of current function's stack.
  int32_t pointer() const noexcept { return _stack_memory_offset; }

 private:
  RegisterTracker* _reg_tracker;
  // The current pointer to the stack in memory, relative to the top of the
  // stack for this calling frame.
  int32_t _stack_memory_offset{0};
  // The current stack size
  size_t _stack_size{0};
  UnderlyingType _stack;
};

}  // namespace wasmcc::x64
