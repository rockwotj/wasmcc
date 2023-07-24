#pragma once

#include "absl/functional/any_invocable.h"
#include "base/type_traits.h"
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
      const Name&, const BlockType&) = 0;

  /**
   * Run the specified compiled function within the VM's thread and stack.
   *
   * Only one live DynamicComputation is allowed at once.
   *
   * NOTE: The VM **must** outlive the resuling computation.
   */
  virtual runtime::DynamicComputation InvokeDynamic(
      absl::AnyInvocable<void()>) = 0;
};

template <typename Signature>
std::optional<FunctionHandle<Signature>> VM::LookupFunctionHandle(
    const Name& name) {
  auto signature = runtime::detail::SignatureFromNative<Signature>();
  auto compiled = LookupFunctionHandleDynamic(name, std::move(signature));
  if (!compiled) {
    return std::nullopt;
  }
  using ArgTypes = FunctionTraits<Signature>::arg_types;
  using ResultType = FunctionTraits<Signature>::result_type;
  return FunctionHandle<Signature>(
      [this, compiled = std::move(compiled).value()](ArgTypes&& args) {
        std::unique_ptr<Computation<ResultType>> typed_computation{
            new Computation<ResultType>()};
        typed_computation->_dyn = InvokeDynamic(
            [compiled = compiled, args = std::forward<ArgTypes>(args),
             comp = typed_computation.get()]() mutable {
              comp->_result =
                  compiled.template apply<Signature, ArgTypes>(std::move(args));
            });

        return std::move(typed_computation);
      });
}

}  // namespace wasmcc
