#include "base/bytes.h"
#include "base/stream.h"
#include "gtest/gtest.h"
#include "leb128/leb128.h"

#include <gtest/gtest.h>

#include <initializer_list>
#include <limits>
#include <vector>

namespace wasmcc::leb128 {

template<typename T>
struct TestData {
    T decoded;
    bytes encoded;
};

template<typename T>
void RunTest(const TestData<T>& testcase) {
    EXPECT_EQ(encode<T>(testcase.decoded), testcase.encoded)
      << "encoding: " << testcase.decoded;

    auto s = ByteStream(testcase.encoded);
    EXPECT_EQ(decode<T>(&s), testcase.decoded)
      << "decoding: " << testcase.decoded;
}

template<typename T>
class RoundTrip : public testing::TestWithParam<TestData<T>> {};

class SignedInt32RoundTrip : public RoundTrip<int32_t> {};
class UnsignedInt32RoundTrip : public RoundTrip<uint32_t> {};
class SignedInt64RoundTrip : public RoundTrip<int64_t> {};
class UnsignedInt64RoundTrip : public RoundTrip<uint64_t> {};

TEST_P(SignedInt32RoundTrip, RoundTrip) { RunTest(GetParam()); }
TEST_P(UnsignedInt32RoundTrip, RoundTrip) { RunTest(GetParam()); }
TEST_P(SignedInt64RoundTrip, RoundTrip) { RunTest(GetParam()); }
TEST_P(UnsignedInt64RoundTrip, RoundTrip) { RunTest(GetParam()); }

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
INSTANTIATE_TEST_SUITE_P(
  InterestingSignedIntegers,
  SignedInt32RoundTrip,
  testing::ValuesIn<std::vector<TestData<int32_t>>>(
    {{.decoded = -165675008, .encoded = {0x80, 0x80, 0x80, 0xb1, 0x7f}},
     {.decoded = -624485, .encoded = {0x9b, 0xf1, 0x59}},
     {.decoded = -16256, .encoded = {0x80, 0x81, 0x7f}},
     {.decoded = -4, .encoded = {0x7c}},
     {.decoded = -1, .encoded = {0x7f}},
     {.decoded = 0, .encoded = {0x00}},
     {.decoded = 1, .encoded = {0x01}},
     {.decoded = 4, .encoded = {0x04}},
     {.decoded = 16256, .encoded = {0x80, 0xff, 0x0}},
     {.decoded = 624485, .encoded = {0xe5, 0x8e, 0x26}},
     {.decoded = 165675008, .encoded = {0x80, 0x80, 0x80, 0xcf, 0x0}},
     {.decoded = std::numeric_limits<int32_t>::max(),
      .encoded = {0xff, 0xff, 0xff, 0xff, 0x7}}}));

INSTANTIATE_TEST_SUITE_P(
  InterestingUnsignedIntegers,
  UnsignedInt32RoundTrip,
  testing::ValuesIn<std::vector<TestData<uint32_t>>>(
    {{.decoded = 0, .encoded = {0x00}},
     {.decoded = 1, .encoded = {0x01}},
     {.decoded = 4, .encoded = {0x04}},
     {.decoded = 16256, .encoded = {0x80, 0x7f}},
     {.decoded = 624485, .encoded = {0xe5, 0x8e, 0x26}},
     {.decoded = 165675008, .encoded = {0x80, 0x80, 0x80, 0x4f}},
     {.decoded = std::numeric_limits<uint32_t>::max(),
      .encoded = {0xff, 0xff, 0xff, 0xff, 0xf}}}));

INSTANTIATE_TEST_SUITE_P(
  InterestingUnsignedLongs,
  UnsignedInt64RoundTrip,
  testing::ValuesIn<std::vector<TestData<uint64_t>>>(
    {{.decoded = 0, .encoded = {0x00}},
     {.decoded = 1, .encoded = {0x01}},
     {.decoded = 4, .encoded = {0x04}},
     {.decoded = 16256, .encoded = {0x80, 0x7f}},
     {.decoded = 624485, .encoded = {0xe5, 0x8e, 0x26}},
     {.decoded = 165675008, .encoded = {0x80, 0x80, 0x80, 0x4f}},
     {.decoded = std::numeric_limits<uint32_t>::max(),
      .encoded = {0xff, 0xff, 0xff, 0xff, 0xf}},
     {.decoded = std::numeric_limits<uint64_t>::max(),
      .encoded = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1}}}));

INSTANTIATE_TEST_SUITE_P(
  InterestingSignedLongs,
  SignedInt64RoundTrip,
  testing::ValuesIn<std::vector<TestData<int64_t>>>({
    {.decoded = -std::numeric_limits<int32_t>::max(),
     .encoded = {0x81, 0x80, 0x80, 0x80, 0x78}},
    {.decoded = -165675008, .encoded = {0x80, 0x80, 0x80, 0xb1, 0x7f}},
    {.decoded = -624485, .encoded = {0x9b, 0xf1, 0x59}},
    {.decoded = -16256, .encoded = {0x80, 0x81, 0x7f}},
    {.decoded = -4, .encoded = {0x7c}},
    {.decoded = -1, .encoded = {0x7f}},
    {.decoded = 0, .encoded = {0x00}},
    {.decoded = 1, .encoded = {0x01}},
    {.decoded = 4, .encoded = {0x04}},
    {.decoded = 16256, .encoded = {0x80, 0xff, 0x0}},
    {.decoded = 624485, .encoded = {0xe5, 0x8e, 0x26}},
    {.decoded = 165675008, .encoded = {0x80, 0x80, 0x80, 0xcf, 0x0}},
    {.decoded = std::numeric_limits<int32_t>::max(),
     .encoded = {0xff, 0xff, 0xff, 0xff, 0x7}},
    {.decoded = std::numeric_limits<int64_t>::max(),
     .encoded = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0}},
  }));

TEST(Overflow, Long) {
    bytes encoded(size_t(11), 0xff);
    auto s = ByteStream(encoded);
    EXPECT_THROW(decode<int64_t>(&s), DecodeException);
    s = ByteStream(encoded);
    EXPECT_THROW(decode<uint64_t>(&s), DecodeException);
}
TEST(Overflow, Int) {
    bytes encoded = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    auto s = ByteStream(encoded);
    EXPECT_THROW(decode<int32_t>(&s), DecodeException);
    s = ByteStream(encoded);
    EXPECT_THROW(decode<uint32_t>(&s), DecodeException);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
} // namespace wasmcc::leb128
