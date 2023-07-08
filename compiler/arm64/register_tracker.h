#pragma once

#include <asmjit/a64.h>

#include <array>
#include <bitset>
#include <optional>

#include "compiler/arm64/call_convention.h"
#include "compiler/common/register_tracker.h"

namespace wasmcc::arm64 {

using RegisterTracker = wasmcc::RegisterTracker<CallingConvention>;

}  // namespace wasmcc::arm64
