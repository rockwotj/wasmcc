#pragma once

#include <asmjit/x86.h>

#include <array>

#include "compiler/common/call_convention.h"

namespace wasmcc::x64 {
using GpReg = asmjit::x86::Gp;
using VecReg = asmjit::x86::Vec;
static constexpr GpReg kSpReg = asmjit::x86::rsp;
// The SystemV calling convention, which is used on Mac + Linux.
struct CallingConvention {
  using GpReg = GpReg;
  // TODO: should we detect and use y/z variants?
  using VecReg = VecReg;
  static constexpr GpReg kSp = kSpReg;
  static constexpr std::array kGpArgs = {
      asmjit::x86::rdi, asmjit::x86::rsi, asmjit::x86::rdx,
      asmjit::x86::rcx, asmjit::x86::r8,  asmjit::x86::r9,
  };
  static constexpr std::array kVecArgs = {
      asmjit::x86::xmm0, asmjit::x86::xmm1, asmjit::x86::xmm2,
      asmjit::x86::xmm3, asmjit::x86::xmm4, asmjit::x86::xmm5,
      asmjit::x86::xmm6, asmjit::x86::xmm7,
  };
  static constexpr std::array kGpRets = {
      asmjit::x86::rax,
      asmjit::x86::rdx,
  };
  static constexpr std::array kVecRets = {
      asmjit::x86::xmm0,
      asmjit::x86::xmm1,
  };
  const static RegisterMask<GpReg> kGpCalleeSavedRegisters;
  const static RegisterMask<VecReg> kVecCalleeSavedRegisters;
  const static RegisterMask<GpReg> kGpCallerSavedRegisters;
  const static RegisterMask<VecReg> kVecCallerSavedRegisters;

  static constexpr size_t kStackAlignment = 16;
};

static_assert(wasmcc::CallingConvention<CallingConvention>,
              "must be a calling convention");

}  // namespace wasmcc::x64
