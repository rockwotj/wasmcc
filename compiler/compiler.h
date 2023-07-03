#include <memory>
#include <random>

#include "base/coro.h"
#include "compiler/function.h"
#include "core/ast.h"

#pragma once

namespace wasmcc {

/**
 * An abstract class that converts our IR into machine code.
 */
class Compiler {
 public:
  /**
   * Create a compiler using a FunctionCompiler for the current platform.
   */
  static std::unique_ptr<Compiler> CreateNative();

  Compiler() = default;
  Compiler(const Compiler&) = delete;
  Compiler& operator=(const Compiler&) = delete;
  Compiler(Compiler&&) = default;
  Compiler& operator=(Compiler&&) = default;
  virtual ~Compiler() = default;

  /**
   * Compile a parsed function into machine code for a target architecture.
   *
   * NOTE: A compiled function's lifetime is currently managed by the compiler
   * that created it, but the memory management may change in the future.
   */
  virtual co::Future<CompiledFunction> Compile(Function) = 0;

  /**
   * Free the memory associated with a compiled function.
   */
  virtual void Release(CompiledFunction) = 0;
};

}  // namespace wasmcc
