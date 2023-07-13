#include "compiler/module.h"

#include "core/ast.h"

namespace wasmcc {
CompiledFunction::CompiledFunction(void* p, Function::Metadata m)
    : _ptr(p), _meta(std::move(m)) {}

void* CompiledFunction::get() const { return _ptr; }
const Function::Metadata& CompiledFunction::metadata() const { return _meta; }
}  // namespace wasmcc
