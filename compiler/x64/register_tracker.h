#pragma once

#include <asmjit/asmjit.h>

#include <array>
#include <bitset>
#include <optional>

namespace wasmcc::x64 {

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
      // Function argument #1 in 64-bit Linux.
      asmjit::x86::rdi,
      // Function argument #2 in 64-bit Linux.
      asmjit::x86::rsi,
      // Function argument #3 in 64-bit Linux.
      asmjit::x86::rdx,
      // Function argument #4 in 64-bit Linux.
      asmjit::x86::rcx,
      // Function argument #5 in 64-bit Linux.
      asmjit::x86::r8,
      // Function argument #6 in 64-bit Linux.
      asmjit::x86::r9,
      asmjit::x86::r10,
      asmjit::x86::r11,
      // Values are returned from functions in this register.
      asmjit::x86::rax,
  };
  static constexpr size_t kNumGeneralPurposeRegisters = 16;
  using RegisterUsedMask = std::bitset<kNumGeneralPurposeRegisters>;

  std::optional<asmjit::x86::Gp> TakeUnusedRegister();

  void MarkRegisterUnused(const asmjit::x86::Gp&);

 private:
  RegisterUsedMask _registers_used_mask;
};
}  // namespace wasmcc::x64
