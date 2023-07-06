#include "compiler/compiler.h"

#include <asmjit/asmjit.h>

#include <memory>
#include <variant>

#include "base/coro.h"
#include "compiler/arm64/compiler.h"
#include "compiler/common/util.h"
#include "compiler/function.h"
#include "compiler/x64/compiler.h"

namespace wasmcc {
namespace {
template <typename T>
class CompilerImpl : public Compiler {
 public:
  CompilerImpl() {
    Check(_code_holder.init(_runtime.environment(), _runtime.cpuFeatures()));
  };

  co::Future<CompiledFunction> Compile(Function func) override {
    T func_compiler(func.meta, &_code_holder);
    func_compiler.Prologue();
    for (const auto& expr : func.body) {
      std::visit(func_compiler, expr);
      co_await co::MaybeYield();
    }
    func_compiler.Epilogue();
    void* compiled = nullptr;
    Check(_runtime.add(&compiled, &_code_holder));
    co_return CompiledFunction(compiled);
  }

  co::Future<CompiledModule> Compile(ParsedModule parsed) override {
    CompiledModule compiled{.exported_functions = parsed.exported_functions};
    compiled.functions.reserve(parsed.functions.size());
    for (auto& func : parsed.functions) {
      compiled.functions.push_back(co_await Compile(std::move(func)));
    }
    co_return std::move(compiled);
  }

  void Release(CompiledFunction compiled) override {
    _runtime.release(compiled.get());
  }

  co::Future<> Release(CompiledModule compiled) override {
    for (const auto& func : compiled.functions) {
      Release(func);
      co_await co::MaybeYield();
    }
  }

 private:
  asmjit::JitRuntime _runtime;
  asmjit::CodeHolder _code_holder;
};
}  // namespace

std::unique_ptr<Compiler> Compiler::CreateNative() {
  auto env = asmjit::Environment::host();
  switch (env.arch()) {
    case asmjit::Arch::kX64:
      return std::make_unique<CompilerImpl<x64::Compiler>>();
    case asmjit::Arch::kAArch64:
      return std::make_unique<CompilerImpl<arm64::Compiler>>();
    default:
      // TODO: Properly crash
      __builtin_unreachable();
  }
}

}  // namespace wasmcc
