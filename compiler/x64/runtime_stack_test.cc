#include "compiler/x64/runtime_stack.h"

#include <gtest/gtest-matchers.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <functional>
#include <iterator>
#include <optional>
#include <vector>

#include "core/value.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace wasmcc::x64 {

constexpr size_t kDefaultStackElements = 32;

TEST(RuntimeStack, EmptyIterator) {
  RegisterTracker rt;
  RuntimeStack s(kDefaultStackElements, &rt);
  EXPECT_THAT(s.ReverseIterator(), testing::IsEmpty());
}

TEST(RuntimeStack, Push) {
  RegisterTracker rt;
  RuntimeStack s(kDefaultStackElements, &rt);
  auto pushed = s.Push({.type = ValType::kI64});
  EXPECT_EQ(s.pointer(), sizeof(int64_t));
  EXPECT_EQ(pushed->stack_pointer, s.pointer());
  EXPECT_THAT(s.ReverseIterator(), testing::ElementsAre(*pushed));
}

TEST(RuntimeStack, Pop) {
  RegisterTracker rt;
  RuntimeStack s(kDefaultStackElements, &rt);
  s.Push({.type = ValType::kI64});
  auto popped = s.Pop();
  EXPECT_EQ(popped.type, ValType::kI64);
  EXPECT_EQ(s.pointer(), 0);
  EXPECT_THAT(s.ReverseIterator(), testing::IsEmpty());
}

}  // namespace wasmcc::x64
