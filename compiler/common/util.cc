#include "compiler/common/util.h"

#include "compiler/common/exception.h"

namespace wasmcc {
void Check(asmjit::Error error) {
  if (error) [[unlikely]] {
    throw CompilationException(asmjit::DebugUtils::errorAsString(error));
  }
}

void ThrowingErrorHandler::handleError(asmjit::Error, const char* message,
                                       asmjit::BaseEmitter*) {
  throw CompilationException(message);
}
}  // namespace wasmcc
