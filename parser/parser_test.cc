#include "parser/parser.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <initializer_list>

#include "base/stream.h"
#include "gmock/gmock.h"
#include "testing/wat.h"

namespace wasmcc {

TEST(Parsing, AddFunc) {
  std::string_view wat = R"WAT(
    (module
      (func $add (param $lhs i32) (param $rhs i32) (result i32)
        local.get $lhs
        local.get $rhs
        i32.add)
      (export "add" (func $add))
    )
  )WAT";
  ByteStream s(Wat2Wasm(wat));
  auto parsed = ParseModule(&s).get();
  EXPECT_FALSE(s.HasRemaining());
  EXPECT_EQ(parsed.functions.size(), 1);
  using ::testing::Pair;
  using ::testing::UnorderedElementsAre;
  EXPECT_THAT(parsed.exported_functions,
              UnorderedElementsAre(Pair(Name("add"), FuncIdx(0))));
}
}  // namespace wasmcc
