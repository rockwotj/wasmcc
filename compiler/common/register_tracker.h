#pragma once

#include <asmjit/a64.h>

#include <array>
#include <bitset>
#include <optional>

#include "compiler/common/call_convention.h"

namespace wasmcc {

/**
 * This class tracks if a given register is in use.
 *
 * In the future this class will also likely need to track **where** a register
 * is in use. So we don't have to do things like always save arguments to the
 * stack.
 */
template <CallingConvention CC>
class RegisterTracker {
 public:
  RegisterTracker() = default;
  RegisterTracker(const RegisterTracker&) = delete;
  RegisterTracker& operator=(const RegisterTracker&) = delete;
  RegisterTracker(RegisterTracker&&) = delete;
  RegisterTracker& operator=(RegisterTracker&&) = delete;
  ~RegisterTracker() = default;

  std::optional<typename CC::GpReg> TakeUnusedRegister();

  void MarkRegisterUnused(const typename CC::GpReg&);

 private:
  RegisterMask<typename CC::GpReg> _gp_reg_mask;
};

template <CallingConvention CC>
std::optional<typename CC::GpReg> RegisterTracker<CC>::TakeUnusedRegister() {
  for (const typename CC::GpReg& reg : CC::kGpCallerSavedRegisters) {
    if (!_gp_reg_mask.Test(reg)) {
      _gp_reg_mask.Set(reg);
      return reg;
    }
  }
  // TODO: Support using (and spilling) callee saved regs
  return std::nullopt;
}

template <CallingConvention CC>
void RegisterTracker<CC>::MarkRegisterUnused(const typename CC::GpReg& reg) {
  _gp_reg_mask.Reset(reg);
}

}  // namespace wasmcc
