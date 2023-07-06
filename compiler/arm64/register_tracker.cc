#include "compiler/arm64/register_tracker.h"

#include <cstddef>

namespace wasmcc::arm64 {

std::optional<asmjit::a64::Gp> RegisterTracker::TakeUnusedRegister() {
  for (const asmjit::a64::Gp& reg : kCallerSavedRegisters) {
    if (!_registers_used_mask.test(reg.id())) {
      _registers_used_mask.set(reg.id());
      return reg;
    }
  }
  return std::nullopt;
}

void RegisterTracker::MarkRegisterUnused(const asmjit::a64::Gp& reg) {
  _registers_used_mask.reset(reg.id());
}
}  // namespace wasmcc::arm64
