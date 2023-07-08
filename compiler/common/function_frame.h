#pragma once
#include "base/align.h"
#include "compiler/common/call_convention.h"
#include "core/ast.h"

namespace wasmcc {

/**
 *
 * The function ahead of time knows it's memory layout thanks to how WASM is
 * designed. We'll reserve the max stack depth (TODO we should also remove
 * pumping the stack when taking into account the available registers). And also
 * reserve space on the stack for persisting locals as well.
 *
 * The locals are stored closer to the top of the stack, so that values that are
 * passed on the stack don't have to be moved. The stack is then placed after
 * that, and stack values grow *towards* the locals (as that was simpler to
 * implement).
 *
 * Here is a graphical representation of the stack usage.
 *
 *  ┌──────────┬───────────────┐
 *  │  LOCALS  │  STACK        │
 *  └──────────┴───────────────┘
 *  0xFFFF                0xFF00
 */
template <CallingConvention CC>
class FunctionFrame {
 public:
  explicit FunctionFrame(Function::Metadata meta);

  /* The size of the stack. */
  int32_t StackSizeBytes() const noexcept {
    // TODO: This can be optimized, as there are registers around to hold the
    // stack size.
    return AlignUp<uint32_t>(_locals_size_bytes + _meta.max_stack_size_bytes,
                             CC::kStackAlignment);
  }

  int32_t LocalStackOffset(size_t idx) const {
    return _locals_stack_offset[idx];
  }

 private:
  Function::Metadata _meta;
  // The size of the locals in bytes.
  int32_t _locals_size_bytes{0};
  // A mapping between a local and it's memory offset onto the stack.
  //
  // The offset is relative to rsp.
  absl::FixedArray<int32_t> _locals_stack_offset;
};

template <CallingConvention CC>
FunctionFrame<CC>::FunctionFrame(Function::Metadata meta)
    : _meta(std::move(meta)),
      _locals_stack_offset(_meta.locals.size() +
                           _meta.signature.parameter_types.size()) {
  // Initially record the stack offset relative to the bottom of the stack
  size_t i = 0;
  for (const auto& type : _meta.signature.parameter_types) {
    _locals_stack_offset[i++] = _locals_size_bytes;
    _locals_size_bytes += int32_t(ValTypeSizeBytes(type));
  }
  for (const auto& type : _meta.locals) {
    _locals_stack_offset[i++] = _locals_size_bytes;
    _locals_size_bytes += int32_t(ValTypeSizeBytes(type));
  }
  // Now that we've calculated the full stack size, adjust the offset to be
  // relative to the top of the stack.
  for (i = 0; i < _locals_stack_offset.size(); ++i) {
    _locals_stack_offset[i] -= StackSizeBytes();
  }
}

}  // namespace wasmcc
