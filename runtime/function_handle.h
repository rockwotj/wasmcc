#pragma once
#include <memory>
#include <tuple>
#include <utility>

#include "absl/functional/any_invocable.h"
#include "base/type_traits.h"
#include "compiler/module.h"

namespace wasmcc {

template <typename Result>
class Computation;

/**
 * A handle to a VM defined function.
 *
 * These can be looked up a head of time and repeated invoked. There is a very
 * small setup cost to actually creating a handle, but most work is deferred to
 * at that time.
 *
 * You may only invoke a single FunctionHandle at once, and must either call the
 * resulting `Computation's` `Execute` method until `IsDone()` is true or call
 * `Stop()`.
 */
template <typename Signature>
class FunctionHandle {
  using ResultType = FunctionTraits<Signature>::result_type;
  using ArgTypes = FunctionTraits<Signature>::arg_types;
  using UnderlyingType =
      absl::AnyInvocable<std::unique_ptr<Computation<ResultType>>(ArgTypes&&)>;

 public:
  explicit FunctionHandle(UnderlyingType u) : _underlying(std::move(u)) {}

  template <typename... Args>
  std::unique_ptr<Computation<ResultType>> Invoke(Args&&... args) {
    return _underlying(std::make_tuple(std::forward<Args>(args)...));
  }

 private:
  UnderlyingType _underlying;
};

namespace runtime {
class VMThread;
/** An untyped version of `Computation`. */
class DynamicComputation {
 public:
  explicit DynamicComputation(VMThread*);
  DynamicComputation(const DynamicComputation&) = delete;
  DynamicComputation& operator=(const DynamicComputation&) = delete;
  DynamicComputation(DynamicComputation&&) noexcept = default;
  DynamicComputation& operator=(DynamicComputation&&) noexcept = default;
  ~DynamicComputation() = default;

  void Execute();
  void Cancel();
  bool IsDone() const noexcept;

 private:
  VMThread* _thread;
};
}  // namespace runtime

/**
 * A running function within the VM.
 *
 * This encapsulates the behavior that the VM can yield execution back to the
 * host, so `Execute()` must continue to be called to yield control back to the
 * VM.
 *
 * If `Execute()` is not called until completion, then `Cancel()` must be
 * called.
 *
 */
template <typename Result>
class Computation {
 public:
  // Computation needs a stable pointer, so it's not moveable or copyable.
  Computation(const Computation&) = delete;
  Computation& operator=(const Computation&) = delete;
  Computation(Computation&&) = delete;
  Computation& operator=(Computation&&) = delete;
  ~Computation() = default;

  // Continue to run the function.
  void Execute() { _dyn.Execute(); }

  // Cancel this computation so another can run or the VM can be shutdown.
  void Cancel() { _dyn.Cancel(); }

  // If this computation has finished executing.
  bool IsDone() const noexcept { return _dyn.IsDone(); }

  // Must wait to call until `IsDone()` returns true.
  Result GetResult() const noexcept { return _result; };

 private:
  friend class VM;

  Computation() : _dyn(nullptr) {}

  runtime::DynamicComputation _dyn;
  // NOTE: This should be a variant when we have traps.
  Result _result{};
};

}  // namespace wasmcc
