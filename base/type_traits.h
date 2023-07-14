#pragma once
#include <tuple>
#include <type_traits>
#include <utility>

namespace wasmcc {
template <typename>
struct DependantFalse : std::false_type {};

template <typename... Args>
concept EmptyPack = sizeof...(Args) == 0;

template <typename T>
struct FunctionTraits : public FunctionTraits<decltype(&T::operator())> {};

template <typename ReturnType, typename... Args>
struct FunctionTraits<ReturnType(Args...)> {
  using result_type = ReturnType;
  using arg_types = std::tuple<Args...>;
};
template <typename ReturnType, typename... Args>
struct FunctionTraits<ReturnType (*)(Args...)> {
  using result_type = ReturnType;
  using arg_types = std::tuple<Args...>;
};

}  // namespace wasmcc
