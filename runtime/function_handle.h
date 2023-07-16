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
 *
 *
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
 * The result of an invoked function.
 */
template <typename Result>
class Computation {
 public:
  // Computation need a stable pointer
  Computation(const Computation&) = delete;
  Computation& operator=(const Computation&) = delete;
  Computation(Computation&&) = delete;
  Computation& operator=(Computation&&) = delete;
  ~Computation() = default;

  void Execute() { _dyn.Execute(); }

  void Cancel() { _dyn.Cancel(); }

  bool IsDone() const noexcept { return _dyn.IsDone(); }

  Result GetResult() const noexcept { return _result; };

 private:
  friend class VM;

  Computation() : _dyn(nullptr) {}

  runtime::DynamicComputation _dyn;
  // NOTE: This should be a variant when we have traps.
  Result _result{};
};

}  // namespace wasmcc
