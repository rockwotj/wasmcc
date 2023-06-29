#include "compiler/x64/runtime_stack.h"

#include <cstdint>

namespace wasmcc::x64 {

size_t RuntimeValue::size_bytes() const {
  switch (type) {
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
      return sizeof(void*);
  }
}

RuntimeStack::RuntimeStack(size_t max_stack_elements, RegisterTracker* tracker)
    : _reg_tracker(tracker), _stack(max_stack_elements) {
  // TODO: We'll need this eventually (I think)
  (void)_reg_tracker;
}

RuntimeValue* RuntimeStack::Push(RuntimeValue v) {
  _stack_memory_offset -= v.size_bytes();
  v.stack_pointer = _stack_memory_offset;
  _stack[_stack_pointer++] = std::move(v);
  return Peek();
}
RuntimeValue RuntimeStack::Pop() {
  auto top = std::move(_stack[_stack_pointer--]);
  _stack_memory_offset += top.size_bytes();
  return top;
}
RuntimeValue* RuntimeStack::Peek() { return &_stack.back(); }
const RuntimeValue* RuntimeStack::Peek() const { return &_stack.back(); }

}  // namespace wasmcc::x64
