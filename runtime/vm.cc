#include "runtime/vm.h"

#include <utility>

namespace wasmcc {
namespace {
class VMImpl final : public VM {
 public:
  explicit VMImpl(CompiledModule compiled) : _compiled(std::move(compiled)) {}

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

 private:
  CompiledModule _compiled;
};
}  // namespace

std::unique_ptr<VM> VM::Create(CompiledModule compiled) {
  return std::make_unique<VMImpl>(std::move(compiled));
}
}  // namespace wasmcc
