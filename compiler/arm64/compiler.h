#pragma once

#include "absl/strings/str_format.h"
#include "compiler/arm64/call_convention.h"
#include "compiler/arm64/register_tracker.h"
#include "compiler/arm64/runtime_stack.h"
#include "compiler/common/function_frame.h"
#include "core/ast.h"
#include "core/instruction.h"

namespace wasmcc::arm64 {

/**
 *
 * The compiler ahead of time knows it's memory layout. We'll reserve the max
 * stack depth (TODO we should also remove pumping the stack when taking into
 * account the available registers). And also reserve space on the stack for
 * persisting locals as well.
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
class Compiler {
 public:
  Compiler(Function::Metadata, asmjit::CodeHolder*);
  Compiler(const Compiler&) = delete;
  Compiler& operator=(const Compiler&) = delete;
  Compiler(Compiler&&) = delete;
  Compiler& operator=(Compiler&&) = delete;
  ~Compiler() = default;

  void SetLogger(asmjit::Logger*);

  void Prologue();
  void Epilogue();

  void operator()(const op::ConstI32&);
  void operator()(const op::AddI32&);
  void operator()(const op::GetLocalI32&);
  void operator()(const op::SetLocalI32&);
  void operator()(const op::Return&);
  void operator()(const op::Label&);
  void operator()(const op::Br&);
  void operator()(const op::BrIf&);

 private:
  GpReg AllocateRegister();
  GpReg EnsureInRegister(RuntimeValue*);

  // Annotate the next instruction emitted
  //
  // The input string must outlive the next instruction emit, and probably
  // should only be static strings.
  void AnnotateNext(const char*);

  // Annotate the next instruction emitted
  //
  // the returned string must be alive when the next instruction is emitted
  // asmjit doesn't free these strings.
  template <typename... A>
  [[nodiscard(
      "the comment must outlie emitting the next instruction")]] std::string
  AnnotateNext(const absl::FormatSpec<A...>& fmt, A&&... args) {
    std::string msg = absl::StrFormat(fmt, std::forward<A>(args)...);
    AnnotateNext(msg.c_str());
    return msg;
  }

  std::unique_ptr<RegisterTracker> _reg_tracker;
  std::unique_ptr<RuntimeStack> _stack;

  Function::Metadata _meta;
  FunctionFrame<CallingConvention> _frame;
  asmjit::a64::Assembler _asm;
  asmjit::Label _exit_label;
};

}  // namespace wasmcc::arm64
