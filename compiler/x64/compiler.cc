#include "compiler/x64/compiler.h"

#include <unistd.h>

#include <cstddef>
#include <memory>

#include "compiler/common/util.h"
#include "compiler/x64/call_convention.h"
#include "compiler/x64/register_tracker.h"
#include "compiler/x64/runtime_stack.h"
#include "core/value.h"

namespace wasmcc::x64 {
namespace {
namespace x86 = asmjit::x86;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static ThrowingErrorHandler kErrorHandler;

}  // namespace

Compiler::Compiler(Function::Metadata meta, asmjit::CodeHolder* holder)
    : _reg_tracker(std::make_unique<RegisterTracker>()),
      _stack(std::make_unique<RuntimeStack>(meta.max_stack_elements)),
      _meta(std::move(meta)),
      _asm(holder),
      _frame(_meta),
      _exit_label(_asm.newLabel()) {
#ifndef NDEBUG
  _asm.addDiagnosticOptions(asmjit::DiagnosticOptions::kValidateAssembler);
#endif
  _asm.setErrorHandler(&kErrorHandler);
}

void Compiler::SetLogger(asmjit::Logger* logger) { _asm.setLogger(logger); }
void Compiler::AnnotateNext(const char* s) { _asm.setInlineComment(s); }

void Compiler::Prologue() {
  AnnotateNext("set locals stack space");
  // rsp -= <stack_size>
  _asm.sub(x86::regs::rsp, _frame.StackSizeBytes());
  // TODO: We should lazily spill these onto the stack, and handle passing
  // values by stack
  for (size_t i = 0; i < _meta.signature.parameter_types.size(); ++i) {
    ValType vt = _meta.signature.parameter_types[i];
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    const auto& reg = Cast(CallingConvention::kGpArgs[i], vt);
    auto comment = AnnotateNext("SaveLocalToStack(%d)", i);
    _asm.mov(x86::Mem(x86::regs::rsp, _frame.LocalStackOffset(i)), reg);
  }
}
void Compiler::Epilogue() {
  AnnotateNext("epilog start");
  _asm.bind(_exit_label);
  if (!_meta.signature.result_types.empty()) {
    auto vt = _meta.signature.result_types.front();
    auto result_reg = Cast(x86::regs::rax, vt);
    auto v = _stack->Pop();
    auto stack_top_reg = EnsureInRegister(&v);
    stack_top_reg = Cast(stack_top_reg, v.type);
    if (stack_top_reg != result_reg) {
      _asm.mov(result_reg, stack_top_reg);
    }
  }
  // rsp += <stack size>
  _asm.add(x86::regs::rsp, _frame.StackSizeBytes());
  _asm.ret();
}

void Compiler::operator()(const op::ConstI32& op) {
  auto* top = _stack->Push({.type = ValType::kI32});
  auto reg = EnsureInRegister(top);
  int32_t v = op.value.AsI32();
  auto comment = AnnotateNext("ConstI32(%d)", v);
  // reg = i32
  _asm.mov(reg.r32(), v);
}
void Compiler::operator()(const op::AddI32&) {
  auto x2 = _stack->Pop();
  auto x2_reg = EnsureInRegister(&x2);
  auto* x1 = _stack->Peek();
  auto x1_reg = EnsureInRegister(x1);
  AnnotateNext("AddI32");
  // x1r += x2r
  _asm.adc(x1_reg.r32(), x2_reg.r32());
  _reg_tracker->MarkRegisterUnused(x2_reg);
}
void Compiler::operator()(const op::GetLocalI32& op) {
  _stack->Push({.type = ValType::kI32});
  auto* top = _stack->Peek();
  top->reg = AllocateRegister();
  auto offset = _frame.LocalStackOffset(op.idx);
  auto comment = AnnotateNext("GetLocalI32(%d)", op.idx);
  _asm.mov(top->reg->r32(), x86::Mem(x86::rsp, offset));
}
void Compiler::operator()(const op::SetLocalI32& op) {
  auto v = _stack->Pop();
  auto v_reg = EnsureInRegister(&v);
  auto offset = _frame.LocalStackOffset(op.idx);
  auto comment = AnnotateNext("SetLocalI32(%d)", op.idx);
  _asm.mov(x86::Mem(x86::rsp, offset), v_reg);
  _reg_tracker->MarkRegisterUnused(v_reg);
}
void Compiler::operator()(const op::Return&) { _asm.jmp(_exit_label); }
void Compiler::operator()(const op::Label&) {
  // TODO
}
void Compiler::operator()(const op::Br&) {
  // TODO
}
void Compiler::operator()(const op::BrIf&) {
  // TODO
}

GpReg Compiler::AllocateRegister() {
  std::optional<GpReg> reg = _reg_tracker->TakeUnusedRegister();
  if (reg) {
    return *reg;
  }
  for (auto& v : _stack->ReverseIterator()) {
    if (!v.reg.has_value()) {
      continue;
    }
    std::swap(reg, v.reg);
    // Spill the register to the stack.
    AnnotateNext("spill onto stack");
    // rsp[sp] = r
    _asm.mov(x86::Mem(x86::rsp, v.stack_pointer), Cast(*reg, v.type));
  }
  ABSL_ASSERT(reg.has_value());
  return *reg;
}

GpReg Compiler::EnsureInRegister(RuntimeValue* v) {
  if (!v->reg) {
    v->reg = AllocateRegister();
    AnnotateNext("load from stack");
    // reg = rsp[sp]
    _asm.mov(Cast(*v->reg, v->type), x86::Mem(x86::rsp, v->stack_pointer));
  }
  return *v->reg;
}

}  // namespace wasmcc::x64
