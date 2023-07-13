#include "runtime/vm.h"

#include <gtest/gtest.h>

#include <cstddef>

#include "base/stream.h"
#include "compiler/compiler.h"
#include "parser/parser.h"
#include "testing/wat.h"

namespace wasmcc {

namespace {

class VMTest : public ::testing::Test {
 public:
  std::unique_ptr<VM> CreateVM(std::string_view wat) {
    auto source = ByteStream(Wat2Wasm(wat));
    auto parsed = ParseModule(&source).get();
    auto compiled = _compiler->Compile(parsed).get();
    return VM::Create(std::move(compiled));
  }

 private:
  std::unique_ptr<Compiler> _compiler = Compiler::CreateNative();
};
}  // namespace

TEST_F(VMTest, Works) {
  auto vm = CreateVM(R"WAT(
  (module
    (func $add (param $lhs i32) (param $rhs i32) (result i32)
      local.get $lhs
      local.get $rhs
      i32.add) (export "add" (func $add)))
  )WAT");
  auto func = vm->LookupFunctionHandle<int (*)(int, int)>(Name("add"));
  ASSERT_NE(func, std::nullopt);
  // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
  EXPECT_EQ(func->invoke(3, 4), 7);
}

}  // namespace wasmcc
