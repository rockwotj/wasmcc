#include "compiler/x64/register_tracker.h"

#include <cstddef>

namespace wasmcc::x64 {

std::optional<asmjit::x86::Gp> RegisterTracker::TakeUnusedRegister() {
  for (const asmjit::x86::Gp& reg : kCallerSavedRegisters) {
    if (!_registers_used_mask.test(reg.id())) {
      _registers_used_mask.set(reg.id());
      return reg;
    }
  }
  return std::nullopt;
}

void RegisterTracker::MarkRegisterUnused(const asmjit::x86::Gp& reg) {
  _registers_used_mask.reset(reg.id());
}
}  // namespace wasmcc::x64
