#include "compiler/arm64/call_convention.h"

namespace wasmcc::arm64 {

const RegisterMask<GpReg> CallingConvention::kGpCalleeSavedRegisters(
    {asmjit::a64::x19, asmjit::a64::x20, asmjit::a64::x21, asmjit::a64::x22,
     asmjit::a64::x23, asmjit::a64::x24, asmjit::a64::x25, asmjit::a64::x26,
     asmjit::a64::x27, asmjit::a64::x28, asmjit::a64::x29});

const RegisterMask<VecReg> CallingConvention::kVecCalleeSavedRegisters(
    {asmjit::a64::v8, asmjit::a64::v9, asmjit::a64::v10, asmjit::a64::v11,
     asmjit::a64::v12, asmjit::a64::v13, asmjit::a64::v14, asmjit::a64::v15});

const RegisterMask<GpReg> CallingConvention::kGpCallerSavedRegisters(
    {// function args
     asmjit::a64::x0, asmjit::a64::x1, asmjit::a64::x2, asmjit::a64::x3,
     asmjit::a64::x4, asmjit::a64::x5, asmjit::a64::x6, asmjit::a64::x7,
     // local variables
     asmjit::a64::x9, asmjit::a64::x10, asmjit::a64::x11, asmjit::a64::x12,
     asmjit::a64::x13, asmjit::a64::x14, asmjit::a64::x15});
const RegisterMask<VecReg> CallingConvention::kVecCallerSavedRegisters(
    {// function args
     asmjit::a64::v0, asmjit::a64::v1, asmjit::a64::v2, asmjit::a64::v3,
     asmjit::a64::v4, asmjit::a64::v5, asmjit::a64::v6, asmjit::a64::v7,
     // local variables
     asmjit::a64::v16, asmjit::a64::v17, asmjit::a64::v18, asmjit::a64::v19,
     asmjit::a64::v20, asmjit::a64::v21, asmjit::a64::v22, asmjit::a64::v23,
     asmjit::a64::v24, asmjit::a64::v25, asmjit::a64::v26, asmjit::a64::v27,
     asmjit::a64::v28, asmjit::a64::v29, asmjit::a64::v30, asmjit::a64::v31});
}  // namespace wasmcc::arm64
