#include "parser/validator.h"

#include <gtest/gtest.h>

#include <initializer_list>
#include <optional>
#include <type_traits>

#include "base/type_traits.h"
#include "core/ast.h"
#include "core/instruction.h"
#include "core/value.h"

namespace wasmcc {
namespace {

template <typename T>
constexpr ValType AsWasmType() {
  if constexpr (std::is_same_v<int, T>) {
    return ValType::kI32;
  } else if constexpr (std::is_same_v<long, T>) {
    return ValType::kI64;
  } else if constexpr (std::is_same_v<float, T>) {
    return ValType::kF32;
  } else if constexpr (std::is_same_v<double, T>) {
    return ValType::kF64;
  } else {
    static_assert(DependantFalse<T>::value, "Unknown wasmtype");
  }
}

template <typename... Rest>
std::vector<ValType> AsWasmTypes()
  requires EmptyPack<Rest...>
{
  return {};
}

template <typename T, typename... R>
std::vector<ValType> AsWasmTypes() {
  std::vector<ValType> v{AsWasmType<T>()};
  for (auto vt : AsWasmTypes<R...>()) {
    v.push_back(vt);
  }
  return v;
}

template <typename R, typename... A>
void check_instructions(const std::initializer_list<Instruction>& ops) {
  BlockType ft{.parameter_types = AsWasmTypes<A...>()};
  if constexpr (!std::is_void_v<R>) {
    auto vt = AsWasmType<R>();
    ft.result_types.push_back(vt);
  }
  auto sv = FunctionValidator(ft, {});
  for (const auto& op : ops) {
    std::visit(sv, op);
  }
  sv.Finalize();
}
}  // namespace

template <typename R, typename... A>
void AssertValid(const std::initializer_list<Instruction>& ops) {
  auto fn = [ops] { check_instructions<R, A...>(ops); };
  EXPECT_NO_THROW(fn());
}
template <typename R, typename... A>
void AssertInvalid(const std::initializer_list<Instruction>& ops) {
  auto fn = [ops] { check_instructions<R, A...>(ops); };
  EXPECT_THROW(fn(), ValidationException);
}

using namespace wasmcc::op;

TEST(Validation, NoopFunc) {
  AssertValid<void>({
      Return(),
  });
}
TEST(Validation, GoodReturnSequence) {
  AssertValid<int>({
      ConstI32(0),
      Return(),
  });
}
TEST(Validation, ImplicitReturn) {
  AssertValid<int>({
      ConstI32(0),
  });
}
TEST(Validation, AddFunc) {
  AssertValid<int, int, int>({
      GetLocalI32(0),
      GetLocalI32(1),
      AddI32(),
      Return(),
  });
}
TEST(Validation, GoodSequence) {
  AssertValid<int>({
      ConstI32(0),
      ConstI32(0),
      AddI32(),
      Return(),
  });
}
TEST(Validation, ExtraStackAtEnd) {
  AssertInvalid<int>({
      ConstI32(0),
      ConstI32(0),
  });
}
TEST(Validation, MissingIntAddSequence) {
  AssertInvalid<int>({
      ConstI32(0),
      AddI32(),
      Return(),
  });
}
TEST(Validation, ExtraIntAddSequence) {
  AssertInvalid<int>({
      ConstI32(0),
      ConstI32(0),
      ConstI32(0),
      AddI32(),
      Return(),
  });
}
TEST(Validation, GetInvalidLocal) {
  AssertInvalid<int>({
      GetLocalI32(0),
      ConstI32(0),
      AddI32(),
      Return(),
  });
}
TEST(Validation, SetInvalidLocal) {
  AssertInvalid<void, int>({
      ConstI32(0),
      ConstI32(0),
      AddI32(),
      SetLocalI32(1),
      Return(),
  });
}
TEST(Validation, SetInvalidLocalType) {
  AssertInvalid<void, long>({
      ConstI32(0),
      SetLocalI32(0),
  });
}
}  // namespace wasmcc
