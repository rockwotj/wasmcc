#include "compiler/function.h"

namespace wasmcc {
CompiledFunction::CompiledFunction(void* p) : _ptr(p) {}

void* CompiledFunction::get() const { return _ptr; }
}  // namespace wasmcc
