#include "compiler/compiler.h"

#include <gtest/gtest.h>

#include "base/stream.h"
#include "compiler/function.h"
#include "parser/parser.h"
#include "testing/wat.h"

namespace wasmcc {
namespace {

class CompilerTest : public ::testing::Test {
 public:
  CompiledModule Compile(std::string_view wat) {
    auto source = ByteStream(Wat2Wasm(wat));
    auto parsed = ParseModule(&source).get();
    return _compiler->Compile(parsed).get();
  }

 private:
  std::unique_ptr<Compiler> _compiler = Compiler::CreateNative();
};
}  // namespace

TEST_F(CompilerTest, CanGenerateAddFn) {
  auto compiled = Compile(R"WAT(
  (module
    (func $add (param $lhs i32) (param $rhs i32) (result i32)
      local.get $lhs
      local.get $rhs
      i32.add) (export "add" (func $add)))
  )WAT");
  auto func_idx = compiled.exported_functions[Name("add")];
  auto add = compiled.functions[func_idx.value()];
  auto result = add.invoke<int32_t, int32_t, int32_t>(1, 2);
  EXPECT_EQ(result, 3);
}

}  // namespace wasmcc
