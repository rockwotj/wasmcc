#pragma once

#include "compiler/module.h"
#include "core/ast.h"
#include "runtime/function_handle.h"
#include "runtime/signature_converter.h"

namespace wasmcc {

/**
 * A VM is an instance of a compiled WASM module.
 *
 * It contains the runtime memory and other needed machinary needed to execute
 * the WASM code.
 */
class VM {
 public:
  VM() = default;
  VM(const VM&) = delete;
  VM& operator=(const VM&) = delete;
  VM(VM&&) = delete;
  VM& operator=(VM&&) = delete;
  virtual ~VM() = default;

  /**
   * Create a VM from the compiled module.
   *
   * TODO: Talk about lifetimes
   */
  static std::unique_ptr<VM> Create(CompiledModule);

  /**
   * Lookup a function handle with the given signature and name.
   *
   * This process will lookup and verify the signature matches, otherwise return
   * nothing.
   *
   * See FunctionHandle's documentation to learn more about how to use the
   * resulting handle.
   *
   * LIFETIMES: The VM must outlive the returned FunctionHandle handle.
   */
  template <typename Signature>
  std::optional<FunctionHandle<Signature>> LookupFunctionHandle(const Name&);

 protected:
  /**
   * Dynamically lookup a function with the given signature.
   */
  virtual std::optional<CompiledFunction> LookupFunctionHandleDynamic(
      const Name&, const FunctionSignature&) = 0;
};

template <typename Signature>
std::optional<FunctionHandle<Signature>> VM::LookupFunctionHandle(
    const Name& name) {
  auto signature = runtime::detail::SignatureFromNative<Signature>();
  auto compiled = LookupFunctionHandleDynamic(name, std::move(signature));
  if (!compiled) {
    return std::nullopt;
  }
  return FunctionHandle<Signature>(std::move(compiled).value());
}

}  // namespace wasmcc
