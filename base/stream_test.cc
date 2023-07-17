#include "base/stream.h"

#include <gtest/gtest.h>

#include <exception>

#include "base/bytes.h"

namespace wasmcc {

TEST(ByteStream, ReadOne) {
  ByteStream stream({
      0x04,
      0x03,
      0x02,
      0x01,
      0x00,
  });
  EXPECT_TRUE(stream.HasRemaining());
  for (int i = 4; i >= 0; --i) {
    EXPECT_EQ(stream.ReadByte(), i);
    EXPECT_EQ(stream.HasRemaining(), i != 0);
    EXPECT_EQ(stream.BytesConsumed(), 5 - i);
  }
  EXPECT_THROW(stream.ReadByte(), EndOfStreamException);
}

TEST(ByteStream, ReadMany) {
  ByteStream stream({
      0x04,
      0x03,
      0x02,
      0x01,
      0x00,
  });
  bytes buf(2, 0);
  EXPECT_EQ(stream.BytesConsumed(), 0);
  EXPECT_TRUE(stream.HasRemaining());
  EXPECT_EQ(stream.ReadBytes(2), bytes({0x04, 0x03}));
  EXPECT_EQ(stream.BytesConsumed(), 2);
  EXPECT_TRUE(stream.HasRemaining());
  EXPECT_EQ(stream.ReadBytes(2), bytes({0x02, 0x01}));
  EXPECT_EQ(stream.BytesConsumed(), 4);
  EXPECT_TRUE(stream.HasRemaining());
  EXPECT_EQ(stream.ReadBytes(1), bytes({0x00}));
  EXPECT_EQ(stream.BytesConsumed(), 5);
  EXPECT_FALSE(stream.HasRemaining());
  EXPECT_THROW(stream.ReadBytes(2), EndOfStreamException);
  EXPECT_EQ(stream.BytesConsumed(), 5);
  EXPECT_FALSE(stream.HasRemaining());
  EXPECT_THROW(stream.ReadByte(), EndOfStreamException);
}

TEST(ByteStream, Skip) {
  ByteStream stream({
      0x04,
      0x03,
      0x02,
      0x01,
      0x00,
  });
  EXPECT_TRUE(stream.HasRemaining());
  EXPECT_EQ(stream.BytesConsumed(), 0);
  stream.Skip(2);
  EXPECT_TRUE(stream.HasRemaining());
  EXPECT_EQ(stream.BytesConsumed(), 2);
  stream.Skip(2);
  EXPECT_TRUE(stream.HasRemaining());
  EXPECT_EQ(stream.BytesConsumed(), 4);
  EXPECT_THROW(stream.Skip(2), EndOfStreamException);
  EXPECT_TRUE(stream.HasRemaining());
  EXPECT_EQ(stream.BytesConsumed(), 4);
  stream.Skip(1);
  EXPECT_FALSE(stream.HasRemaining());
  EXPECT_EQ(stream.BytesConsumed(), 5);
}

}  // namespace wasmcc
