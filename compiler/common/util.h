#pragma once

#include <asmjit/asmjit.h>

namespace wasmcc {

/**
 * Check if there was error and throw if there was.
 */
void Check(asmjit::Error error);

class ThrowingErrorHandler final : public asmjit::ErrorHandler {
  void handleError(asmjit::Error, const char*, asmjit::BaseEmitter*) final;
};
}  // namespace wasmcc
