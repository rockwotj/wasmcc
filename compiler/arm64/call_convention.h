#pragma once

#include <asmjit/a64.h>

#include <array>
#include <initializer_list>

#include "compiler/common/call_convention.h"
#include "core/value.h"

namespace wasmcc::arm64 {
using GpReg = asmjit::a64::Gp;
using VecReg = asmjit::a64::Vec;
constexpr GpReg kSpReg = asmjit::a64::sp;

GpReg Cast(const GpReg& reg, ValType vt);

// The AArch64 calling convention, it's the same on all platforms 🙌
struct CallingConvention {
  using GpReg = GpReg;
  using VecReg = VecReg;
  static constexpr GpReg kSp = kSpReg;
  static constexpr std::array kGpArgs = {
      asmjit::a64::x0, asmjit::a64::x1, asmjit::a64::x2, asmjit::a64::x3,
      asmjit::a64::x4, asmjit::a64::x5, asmjit::a64::x6, asmjit::a64::x7,
  };
  static constexpr std::array kVecArgs = {
      asmjit::a64::v0, asmjit::a64::v1, asmjit::a64::v2, asmjit::a64::v3,
      asmjit::a64::v4, asmjit::a64::v5, asmjit::a64::v6, asmjit::a64::v7,
  };
  static constexpr std::array kGpRets = kGpArgs;
  static constexpr std::array kVecRets = kVecArgs;
  const static RegisterMask<GpReg> kGpCalleeSavedRegisters;
  // NOTE: only the bottom 64 bits are preserved.
  const static RegisterMask<VecReg> kVecCalleeSavedRegisters;
  const static RegisterMask<GpReg> kGpCallerSavedRegisters;
  const static RegisterMask<VecReg> kVecCallerSavedRegisters;

  static constexpr size_t kStackAlignment = 16;
};

}  // namespace wasmcc::arm64
