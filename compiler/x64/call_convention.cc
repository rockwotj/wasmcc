#include "compiler/x64/call_convention.h"

namespace wasmcc::x64 {

GpReg Cast(const GpReg& reg, ValType vt) {
  GpReg reg32 = reg.r32();
  GpReg reg64 = reg.r64();
  return IsValType32Bit(vt) ? reg32 : reg64;
}
const RegisterMask<GpReg> CallingConvention::kGpCalleeSavedRegisters({
    asmjit::x86::rbx,
    asmjit::x86::rbp,
    asmjit::x86::r12,
    asmjit::x86::r13,
    asmjit::x86::r14,
    asmjit::x86::r15,
});
const RegisterMask<VecReg> CallingConvention::kVecCalleeSavedRegisters({});
const RegisterMask<GpReg> CallingConvention::kGpCallerSavedRegisters({
    asmjit::x86::rax,
    asmjit::x86::rcx,
    asmjit::x86::rdx,
    asmjit::x86::rsi,
    asmjit::x86::rdi,
    asmjit::x86::r8,
    asmjit::x86::r9,
    asmjit::x86::r10,
    asmjit::x86::r11,
});
const RegisterMask<VecReg> CallingConvention::kVecCallerSavedRegisters({
    asmjit::x86::xmm0,  asmjit::x86::xmm1,  asmjit::x86::xmm2,
    asmjit::x86::xmm3,  asmjit::x86::xmm4,  asmjit::x86::xmm5,
    asmjit::x86::xmm6,  asmjit::x86::xmm7,  asmjit::x86::xmm8,
    asmjit::x86::xmm9,  asmjit::x86::xmm10, asmjit::x86::xmm11,
    asmjit::x86::xmm12, asmjit::x86::xmm13, asmjit::x86::xmm14,
    asmjit::x86::xmm15, asmjit::x86::xmm16, asmjit::x86::xmm17,
    asmjit::x86::xmm18, asmjit::x86::xmm19, asmjit::x86::xmm20,
    asmjit::x86::xmm21, asmjit::x86::xmm22, asmjit::x86::xmm23,
    asmjit::x86::xmm24, asmjit::x86::xmm25, asmjit::x86::xmm26,
    asmjit::x86::xmm27, asmjit::x86::xmm28, asmjit::x86::xmm29,
    asmjit::x86::xmm30, asmjit::x86::xmm31,
});
}  // namespace wasmcc::x64
