#include "parser/parser.h"

#include <gtest/gtest.h>

#include <initializer_list>

#include "base/stream.h"

namespace wasmcc {

TEST(Parsing, AddFunc) {
  // |bytes| contains the binary format for the following module:
  //
  //     (func (export "add") (param i32 i32) (result i32)
  //       get_local 0
  //       get_local 1
  //       i32.add)
  //
  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
  bytes buf{0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x07, 0x01,
            0x60, 0x02, 0x7f, 0x7f, 0x01, 0x7f, 0x03, 0x02, 0x01, 0x00, 0x07,
            0x07, 0x01, 0x03, 0x61, 0x64, 0x64, 0x00, 0x00, 0x0a, 0x09, 0x01,
            0x07, 0x00, 0x20, 0x00, 0x20, 0x01, 0x6a, 0x0b};
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
  ByteStream s(std::move(buf));
  auto parsed = ParseModule(&s).get();
  EXPECT_EQ(parsed.functions.size(), 1);
}
}  // namespace wasmcc
