#pragma once

#include "compiler/arm64/call_convention.h"
#include "compiler/common/runtime_stack.h"

namespace wasmcc::arm64 {

using RuntimeValue = RuntimeValue<CallingConvention>;
using RuntimeStack = RuntimeStack<CallingConvention>;

}  // namespace wasmcc::arm64
