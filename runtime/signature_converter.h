#pragma once

#include <cstdint>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "base/type_traits.h"
#include "core/ast.h"
#include "core/value.h"

namespace wasmcc::runtime::detail {

template <typename Value>
consteval ValType NativeToWasm() {
  if constexpr (std::is_same_v<Value, int32_t> ||
                std::is_same_v<Value, uint32_t>) {
    return ValType::kI32;
  } else if constexpr (std::is_same_v<Value, int64_t> ||
                       std::is_same_v<Value, uint64_t>) {
    return ValType::kI64;
  } else if constexpr (std::is_same_v<Value, float>) {
    return ValType::kF32;
  } else if constexpr (std::is_same_v<Value, double>) {
    return ValType::kF64;
  } else {
    static_assert(DependantFalse<Value>::value, "Unsupported wasm type");
  }
}

template <size_t I, typename Tuple>
void AppendConvertedWasmType(std::vector<ValType>* output) {
  if constexpr (I < std::tuple_size_v<Tuple>) {
    output->push_back(NativeToWasm<std::tuple_element_t<I, Tuple>>());
    AppendConvertedWasmType<I + 1, Tuple>(output);
  }
}

template <typename Tuple>
std::vector<ValType> ConvertNativeSignatureToWasm() {
  std::vector<ValType> converted;
  converted.reserve(std::tuple_size_v<Tuple>);
  AppendConvertedWasmType<0, Tuple>(&converted);
  return converted;
}

/**
 * Convert a native C++ function signature into a wasm signature at compile
 * time. (well the vector isn't created at compile time as we need a constexpr
 * container, but otherwise could be done).
 */
template <typename Signature>
BlockType SignatureFromNative() {
  using Func = FunctionTraits<Signature>;
  BlockType sig;
  sig.parameter_types =
      ConvertNativeSignatureToWasm<typename Func::arg_types>();
  if constexpr (!std::is_void_v<typename Func::result_type>) {
    sig.result_types =
        ConvertNativeSignatureToWasm<std::tuple<typename Func::result_type>>();
  }
  return sig;
}

}  // namespace wasmcc::runtime::detail
