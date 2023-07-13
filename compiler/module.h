#pragma once
#include <functional>
#include <string>
#include <tuple>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "core/ast.h"

namespace wasmcc {
/**
 * A strongly typed wrapper around dynamically created code.
 */
class CompiledFunction {
 public:
  CompiledFunction(void*, Function::Metadata);

  template <typename Fn, typename Args>
  decltype(auto) apply(Args&& args) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto typed_ptr = reinterpret_cast<Fn>(_ptr);
    return std::apply(typed_ptr, std::forward<Args>(args));
  }

  template <typename R, typename... A>
  R invoke(A&&... args) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto typed_ptr = reinterpret_cast<R (*)(A...)>(_ptr);
    return std::invoke(typed_ptr, std::forward<A>(args)...);
  }

  void* get() const;

  const Function::Metadata& metadata() const;

 private:
  void* _ptr;
  Function::Metadata _meta;
};

struct CompiledModule {
  std::vector<CompiledFunction> functions;
  absl::flat_hash_map<Name, FuncIdx> exported_functions;
};

}  // namespace wasmcc
