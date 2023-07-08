#pragma once

#include <asmjit/asmjit.h>

#include <array>
#include <bitset>
#include <optional>

#include "compiler/common/register_tracker.h"
#include "compiler/x64/call_convention.h"

namespace wasmcc::x64 {

using RegisterTracker = RegisterTracker<CallingConvention>;

}  // namespace wasmcc::x64
