#include "compiler/common/exception.h"

namespace wasmcc {
CompilationException::CompilationException(std::string msg)
    : _msg(std::move(msg)) {}

const char* CompilationException::what() const noexcept { return _msg.c_str(); }
}  // namespace wasmcc
