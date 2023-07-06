#pragma once

#include <asmjit/a64.h>

#include <array>
#include <bitset>
#include <optional>

namespace wasmcc::arm64 {

/**
 * This class tracks if a given register is in use.
 *
 * In the future this class will also likely need to track **where** a register
 * is in use. So we don't have to do things like always save arguments to the
 * stack.
 */
class RegisterTracker {
 public:
  RegisterTracker() = default;
  RegisterTracker(const RegisterTracker&) = delete;
  RegisterTracker& operator=(const RegisterTracker&) = delete;
  RegisterTracker(RegisterTracker&&) = delete;
  RegisterTracker& operator=(RegisterTracker&&) = delete;
  ~RegisterTracker() = default;

  /**
   * Currently, we only use caller saved registers, but we should certainly also
   * be able to use save callee registers too to resolve some register pressure.
   */
  static constexpr std::array kCallerSavedRegisters = {
      // function args and results
      asmjit::a64::regs::x0,
      asmjit::a64::regs::x1,
      asmjit::a64::regs::x2,
      asmjit::a64::regs::x3,
      asmjit::a64::regs::x4,
      asmjit::a64::regs::x5,
      asmjit::a64::regs::x6,
      asmjit::a64::regs::x7,
      // caller saved
      asmjit::a64::regs::x9,
      asmjit::a64::regs::x10,
      asmjit::a64::regs::x11,
      asmjit::a64::regs::x12,
      asmjit::a64::regs::x13,
      asmjit::a64::regs::x14,
      asmjit::a64::regs::x15,
  };
  static constexpr size_t kNumGeneralPurposeRegisters = 32;
  using RegisterUsedMask = std::bitset<kNumGeneralPurposeRegisters>;

  std::optional<asmjit::a64::Gp> TakeUnusedRegister();

  void MarkRegisterUnused(const asmjit::a64::Gp&);

 private:
  RegisterUsedMask _registers_used_mask;
};
}  // namespace wasmcc::arm64
