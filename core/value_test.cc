#include "core/value.h"

#include <gtest/gtest.h>

#include <cmath>
#include <limits>

namespace wasmcc {

void TestRoundtrip(int32_t v) {
  Value value = Value::I32(v);
  EXPECT_EQ(value.AsI32(), v);
}

TEST(ValueTest, I32) {
  using limits = std::numeric_limits<int32_t>;
  TestRoundtrip(limits::min());
  TestRoundtrip(-limits::max());
  TestRoundtrip(-1);
  TestRoundtrip(0);
  TestRoundtrip(1);
  TestRoundtrip(limits::max());
}

void TestRoundtrip(uint32_t v) {
  Value value = Value::U32(v);
  EXPECT_EQ(value.AsU32(), v);
}

TEST(ValueTest, U32) {
  using limits = std::numeric_limits<uint32_t>;
  TestRoundtrip(limits::min());
  TestRoundtrip(std::numeric_limits<int32_t>::max());
  TestRoundtrip(limits::max());
}
void TestRoundtrip(int64_t v) {
  Value value = Value::I64(v);
  EXPECT_EQ(value.AsI64(), v);
}

TEST(ValueTest, I64) {
  using limits = std::numeric_limits<int64_t>;
  TestRoundtrip(limits::min());
  TestRoundtrip(-std::numeric_limits<int32_t>::max());
  TestRoundtrip(-std::numeric_limits<uint32_t>::max());
  TestRoundtrip(std::numeric_limits<int32_t>::min());
  TestRoundtrip(std::numeric_limits<uint32_t>::min());
  TestRoundtrip(std::numeric_limits<int32_t>::max());
  TestRoundtrip(std::numeric_limits<uint32_t>::max());
  TestRoundtrip(limits::max());
}
void TestRoundtrip(double v) {
  Value value = Value::F64(v);
  if (std::isnan(v)) {
    EXPECT_TRUE(std::isnan(value.AsF64()));
    EXPECT_EQ(std::signbit(value.AsF64()), std::signbit(v));
  } else {
    EXPECT_EQ(value.AsF64(), v);
  }
}
TEST(ValueTest, F64) {
  using limits = std::numeric_limits<double>;
  TestRoundtrip(limits::quiet_NaN());
  TestRoundtrip(limits::signaling_NaN());
  TestRoundtrip(-limits::quiet_NaN());
  TestRoundtrip(-limits::signaling_NaN());
  TestRoundtrip(-limits::infinity());
  TestRoundtrip(limits::min());
  TestRoundtrip(std::numeric_limits<int32_t>::min());
  TestRoundtrip(-limits::epsilon());
  TestRoundtrip(-0.0);
  TestRoundtrip(0.0);
  TestRoundtrip(limits::epsilon());
  TestRoundtrip(std::numeric_limits<int32_t>::max());
  TestRoundtrip(std::numeric_limits<uint32_t>::max());
  TestRoundtrip(limits::max());
  TestRoundtrip(limits::infinity());
}
}  // namespace wasmcc
