#pragma once

#include <span>

#include "absl/container/fixed_array.h"
#include "compiler/common/call_convention.h"
#include "compiler/common/register_tracker.h"
#include "core/value.h"

namespace wasmcc {

template <CallingConvention CC>
struct RuntimeValue {
  // The offset relative to the base of the stack in memory where this stack
  // value can spill to if not in a register.
  int32_t stack_pointer = 0;
  // The register that this value is located in.
  std::optional<typename CC::GpReg> reg;
  // The type of this register
  ValType type = ValType::kI32;

  bool operator==(const RuntimeValue&) const = default;

  // The size of this value on the stack in bytes.
  size_t size_bytes() const { return ValTypeSizeBytes(type); }
};

/**
 * This class tracks what is on the stack during compilation.
 *
 * It also is responsible for tracking stack value's location in stack memory.
 */
template <CallingConvention CC>
class RuntimeStack {
  using UnderlyingType = absl::FixedArray<RuntimeValue<CC>>;

 public:
  explicit RuntimeStack(size_t max_stack_elements)
      : _stack(max_stack_elements) {}
  RuntimeStack(const RuntimeStack&) = delete;
  RuntimeStack& operator=(const RuntimeStack&) = delete;
  RuntimeStack(RuntimeStack&&) = delete;
  RuntimeStack& operator=(RuntimeStack&&) = delete;
  ~RuntimeStack() = default;

  RuntimeValue<CC>* Push(RuntimeValue<CC> v) noexcept {
    _stack_memory_offset += int32_t(v.size_bytes());
    v.stack_pointer = _stack_memory_offset;
    _stack[_stack_size++] = std::move(v);
    return Peek();
  }
  [[nodiscard]] RuntimeValue<CC> Pop() noexcept {
    auto top = std::move(_stack[--_stack_size]);
    _stack_memory_offset -= int32_t(top.size_bytes());
    return top;
  }
  RuntimeValue<CC>* Peek() noexcept { return &_stack[_stack_size - 1]; }
  const RuntimeValue<CC>* Peek() const noexcept {
    return &_stack[_stack_size - 1];
  }

  // Iterator from the bottom of the stack to the top.
  std::span<RuntimeValue<CC>> ReverseIterator() noexcept {
    return {_stack.data(), _stack_size};
  }

  // The current offset in bytes from the bottom of current function's stack.
  int32_t pointer() const noexcept { return _stack_memory_offset; }

 private:
  // The current pointer to the stack in memory, relative to the top of the
  // stack for this calling frame.
  int32_t _stack_memory_offset{0};
  // The current stack size
  size_t _stack_size{0};
  UnderlyingType _stack;
};

}  // namespace wasmcc
