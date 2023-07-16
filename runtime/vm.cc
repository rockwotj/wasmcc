#include "runtime/vm.h"

#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>

#include "base/assert.h"
#include "runtime/function_handle.h"
#include "runtime/thread/thread.h"

namespace wasmcc {
namespace runtime {
class VMImpl final : public VM {
 public:
  explicit VMImpl(CompiledModule compiled)
      : _compiled(std::move(compiled)),
        _thread(runtime::VMThread::Create([this] { RunInternal(); }, {})) {}

  std::optional<CompiledFunction> LookupFunctionHandleDynamic(
      const Name& name, const FunctionSignature& signature) final {
    auto it = _compiled.exported_functions.find(name);
    if (it == _compiled.exported_functions.end()) {
      return std::nullopt;
    }
    const auto& compiled = _compiled.functions[it->second.value()];
    if (compiled.metadata().signature != signature) {
      return std::nullopt;
    }
    return compiled;
  }

  DynamicComputation InvokeDynamic(absl::AnyInvocable<void()> fn) final {
    if (_current_fn || _thread->state() != runtime::VMThread::State::kStopped) {
      throw std::runtime_error(
          "cannot run a function when one is already executing.");
    }
    _current_fn = std::move(fn);
    auto comp = runtime::DynamicComputation(_thread.get());
    // We immediately yield, but this ensures that _current_fn is exchanged, so
    // it's possible to immediately throw away the result of the computation.
    _thread->Resume();
    return comp;
  }

 private:
  void RunInternal() {
    // Clear _current function immediately so that there
    // is no issue with _thread->Stop() being able to be reset.
    auto fn = std::exchange(_current_fn, std::nullopt);
    Assert(fn.has_value(), "run_internal called without anything to run");
    // Pause so the computation is ready
    VMThread::Yield();
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    (*fn)();
  }

  std::optional<absl::AnyInvocable<void()>> _current_fn;
  CompiledModule _compiled;
  std::unique_ptr<runtime::VMThread> _thread;
};
}  // namespace runtime

std::unique_ptr<VM> VM::Create(CompiledModule compiled) {
  return std::make_unique<runtime::VMImpl>(std::move(compiled));
}
}  // namespace wasmcc
