#pragma once
#include <tuple>
#include <utility>

#include "base/type_traits.h"
#include "compiler/module.h"

namespace wasmcc {

template <typename Signature>
class FunctionHandle {
 public:
  explicit FunctionHandle(CompiledFunction c) : _compiled(std::move(c)){};

  template <typename... Args>
  decltype(auto) invoke(Args&&... args) {
    return _compiled.apply<Signature>(
        std::make_tuple<Args...>(std::forward<Args>(args)...));
  }

 private:
  CompiledFunction _compiled;
};

}  // namespace wasmcc
