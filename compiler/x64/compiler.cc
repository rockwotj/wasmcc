#include "compiler/x64/compiler.h"

#include <unistd.h>

#include <cstddef>
#include <memory>

#include "compiler/common/util.h"
#include "compiler/x64/register_tracker.h"
#include "compiler/x64/runtime_stack.h"
#include "core/value.h"

namespace wasmcc::x64 {
namespace {
namespace x86 = asmjit::x86;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static ThrowingErrorHandler kErrorHandler;

x86::Gp Cast(const x86::Gp& reg, ValType vt) {
  x86::Gp reg32 = reg.r32();
  x86::Gp reg64 = reg.r64();
  return IsValType32Bit(vt) ? reg32 : reg64;
}

asmjit::TypeId ValTypeToId(ValType vt) {
  switch (vt) {
    case ValType::kI32:
      return asmjit::TypeId::kInt32;
    case ValType::kI64:
      return asmjit::TypeId::kInt64;
    case ValType::kF32:
      return asmjit::TypeId::kFloat32;
    case ValType::kF64:
      return asmjit::TypeId::kFloat64;
    case ValType::kV128:
      return asmjit::TypeId::kInt8x16;
    case ValType::kFuncRef:
    case ValType::kExternRef:
      return asmjit::TypeId::kUIntPtr;
    default:
      __builtin_unreachable();
  }
}
}  // namespace

Compiler::Compiler(Function::Metadata meta, asmjit::CodeHolder* holder)
    : _locals_size_bytes(0),  // To be calculated
      _locals_stack_offset(meta.locals.size() +
                           meta.signature.parameter_types.size()),
      _reg_tracker(std::make_unique<RegisterTracker>()),
      _stack(std::make_unique<RuntimeStack>(meta.max_stack_elements,
                                            _reg_tracker.get())),
      _meta(std::move(meta)),
      _asm(holder),
      _exit_label(_asm.newLabel()) {
#ifndef NDEBUG
  _asm.addDiagnosticOptions(asmjit::DiagnosticOptions::kValidateAssembler);
#endif
  _asm.setErrorHandler(&kErrorHandler);

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
    _locals_stack_offset[i] -=
        _locals_size_bytes + int32_t(_meta.max_stack_size_bytes);
  }
  // NOTE: This only supports upto 16 arguments.
  asmjit::FuncSignatureBuilder b;
  for (ValType vt : _meta.signature.parameter_types) {
    b.addArg(ValTypeToId(vt));
  }
  switch (_meta.signature.result_types.size()) {
    case 0:
      b.setRet(asmjit::TypeId::kVoid);
      break;
    case 1:
      b.addArg(ValTypeToId(_meta.signature.result_types.front()));
    default:
      // A pointer to a structure of result values
      b.setRet(asmjit::TypeId::kUIntPtr);
      break;
  }
  asmjit::FuncDetail func_detail;
  Check(func_detail.init(b, _asm.environment()));
  Check(_frame.init(func_detail));
  // TODO: There are cases where it makes sense to save some caller registers:
  // _frame.setAllDirty(asmjit::RegGroup::kGp);
  Check(_frame.finalize());
  _asm.emitProlog(_frame);
  AnnotateNext("set locals stack space");
  // rsp -= <stack_size>
  _asm.sub(x86::regs::rsp, _meta.max_stack_size_bytes + _locals_size_bytes);
  // TODO: We should lazily spill these onto the stack, and handle passing
  // values by stack
  for (size_t i = 0; i < _meta.signature.parameter_types.size(); ++i) {
    ValType vt = _meta.signature.parameter_types[i];
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    const auto& reg = Cast(RegisterTracker::kCallerSavedRegisters[i], vt);
    auto comment = AnnotateNext("SaveLocalToStack(%d)", i);
    _asm.mov(x86::Mem(x86::regs::rsp, _locals_stack_offset[i]), reg);
  }
}

void Compiler::SetLogger(asmjit::Logger* logger) { _asm.setLogger(logger); }
void Compiler::AnnotateNext(const char* s) { _asm.setInlineComment(s); }

void Compiler::Prologue() {}
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
  _asm.add(x86::regs::rsp, _meta.max_stack_size_bytes + _locals_size_bytes);
  _asm.emitEpilog(_frame);
}

void Compiler::operator()(op::ConstI32 op) {
  auto* top = _stack->Push({.type = ValType::kI32});
  auto reg = EnsureInRegister(top);
  int32_t v = op.value.AsI32();
  auto comment = AnnotateNext("ConstI32(%d)", v);
  // reg = i32
  _asm.mov(reg.r32(), v);
}
void Compiler::operator()(op::AddI32) {
  auto x2 = _stack->Pop();
  auto x2_reg = EnsureInRegister(&x2);
  auto* x1 = _stack->Peek();
  auto x1_reg = EnsureInRegister(x1);
  AnnotateNext("AddI32");
  // x1r += x2r
  _asm.adc(x1_reg.r32(), x2_reg.r32());
  _reg_tracker->MarkRegisterUnused(x2_reg);
}
void Compiler::operator()(op::GetLocalI32 op) {
  _stack->Push({.type = ValType::kI32});
  auto* top = _stack->Peek();
  top->reg = AllocateRegister();
  auto offset = _locals_stack_offset[op.idx];
  auto comment = AnnotateNext("GetLocalI32(%d)", op.idx);
  _asm.mov(top->reg->r32(), x86::Mem(x86::rsp, offset));
}
void Compiler::operator()(op::SetLocalI32 op) {
  auto v = _stack->Pop();
  auto v_reg = EnsureInRegister(&v);
  auto offset = _locals_stack_offset[op.idx];
  auto comment = AnnotateNext("SetLocalI32(%d)", op.idx);
  _asm.mov(x86::Mem(x86::rsp, offset), v_reg);
  _reg_tracker->MarkRegisterUnused(v_reg);
}
void Compiler::operator()(op::Return) { _asm.jmp(_exit_label); }

x86::Gp Compiler::AllocateRegister() {
  std::optional<x86::Gp> reg = _reg_tracker->TakeUnusedRegister();
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

x86::Gp Compiler::EnsureInRegister(RuntimeValue* v) {
  if (!v->reg) {
    v->reg = AllocateRegister();
    AnnotateNext("load from stack");
    // reg = rsp[sp]
    _asm.mov(Cast(*v->reg, v->type), x86::Mem(x86::rsp, v->stack_pointer));
  }
  return *v->reg;
}

}  // namespace wasmcc::x64
