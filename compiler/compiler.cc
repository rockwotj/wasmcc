#include "compiler/compiler.h"

#include <asmjit/asmjit.h>

#include <memory>
#include <source_location>
#include <variant>

#include "base/assert.h"
#include "base/coro.h"
#include "compiler/arm64/compiler.h"
#include "compiler/common/util.h"
#include "compiler/module.h"
#include "compiler/x64/compiler.h"

namespace wasmcc {
namespace {
template <typename T>
class CompilerImpl : public Compiler {
 public:
  CompilerImpl() {
    Check(_code_holder.init(_runtime.environment(), _runtime.cpuFeatures()));
  };

  co::Future<CompiledFunction> Compile(Function func) {
    T func_compiler(func.meta, &_code_holder);
    asmjit::StringLogger logger;
    func_compiler.SetLogger(&logger);
    func_compiler.Prologue();
    for (const auto& expr : func.body) {
      std::visit(func_compiler, expr);
      co_await co::MaybeYield();
    }
    func_compiler.Epilogue();
    void* compiled = nullptr;
    Check(_runtime.add(&compiled, &_code_holder));
    std::cout << logger.data() << std::endl;
    co_return CompiledFunction(compiled, std::move(func.meta));
  }

  co::Future<CompiledModule> Compile(ParsedModule parsed) override {
    CompiledModule compiled{.exported_functions = parsed.exported_functions};
    compiled.functions.reserve(parsed.functions.size());
    for (auto& func : parsed.functions) {
      compiled.functions.push_back(co_await Compile(std::move(func)));
    }
    co_return std::move(compiled);
  }

  void Release(const CompiledFunction& compiled) {
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
  std::string_view unsupported_arch;
  switch (env.arch()) {
    case asmjit::Arch::kX64:
      return std::make_unique<CompilerImpl<x64::Compiler>>();
    case asmjit::Arch::kAArch64:
      return std::make_unique<CompilerImpl<arm64::Compiler>>();
    case asmjit::Arch::kX86:
      unsupported_arch = "x86";
      break;
    case asmjit::Arch::kRISCV32:
      unsupported_arch = "riscv_32";
      break;
    case asmjit::Arch::kRISCV64:
      unsupported_arch = "riscv_64";
      break;
    case asmjit::Arch::kARM:
      unsupported_arch = "arm";
      break;
    case asmjit::Arch::kThumb:
      unsupported_arch = "thumb";
      break;
    case asmjit::Arch::kMIPS32_LE:
      unsupported_arch = "mips32_le";
      break;
    case asmjit::Arch::kMIPS64_LE:
      unsupported_arch = "mips64_le";
      break;
    case asmjit::Arch::kARM_BE:
      unsupported_arch = "arm_be";
      break;
    case asmjit::Arch::kAArch64_BE:
      unsupported_arch = "aarch64_be";
      break;
    case asmjit::Arch::kThumb_BE:
      unsupported_arch = "thumb_be";
      break;
    case asmjit::Arch::kMIPS32_BE:
      unsupported_arch = "mips32_be";
      break;
    case asmjit::Arch::kMIPS64_BE:
      unsupported_arch = "mips64_be";
      break;
    case asmjit::Arch::kUnknown:
      unsupported_arch = "unknown";
      break;
  }
  Abort(std::source_location::current(), "unsupported architecture: %s",
        unsupported_arch);
}

}  // namespace wasmcc
