#pragma once

#include "compiler/module.h"
#include "core/ast.h"
#include "runtime/function_handle.h"
#include "runtime/function_signature.h"

namespace wasmcc {

/**
 *
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
   *
   *
   */
  static std::unique_ptr<VM> Create(CompiledModule);

  /**
   *
   */
  template <typename Signature>
  std::optional<FunctionHandle<Signature>> LookupFunctionHandle(const Name&);

 protected:
  /**
   * Dynamically lookup a function
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
