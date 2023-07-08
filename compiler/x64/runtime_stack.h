#pragma once

#include "compiler/common/runtime_stack.h"
#include "compiler/x64/call_convention.h"

namespace wasmcc::x64 {

using RuntimeValue = RuntimeValue<CallingConvention>;
using RuntimeStack = RuntimeStack<CallingConvention>;

}  // namespace wasmcc::x64
