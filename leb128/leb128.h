#pragma once

#include <climits>
#include <cstdint>

#include "base/bytes.h"
#include "base/stream.h"

namespace wasmcc::leb128 {

template <typename int_type>
bytes encode(int_type value) {
  bytes output;

  static_assert(sizeof(int_type) == sizeof(uint32_t) ||
                    sizeof(int_type) == sizeof(uint64_t),
                "Only 32bit and 64bit integers are supported");
  constexpr unsigned lower_seven_bits_mask = 0x7FU;
  constexpr unsigned leading_bit_mask = 0x80U;
  constexpr unsigned sign_bit_mask = 0x40U;
  if constexpr (std::is_signed_v<int_type>) {
    bool more = true;
    while (more) {
      uint8_t byte = value & lower_seven_bits_mask;
      value >>= CHAR_BIT - 1;
      if (((value == 0) && !(byte & sign_bit_mask)) ||
          ((value == -1) && (byte & sign_bit_mask))) {
        more = false;
      } else {
        byte |= leading_bit_mask;
      }
      output.append(&byte, 1);
    }
  } else {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-do-while)
    do {
      uint8_t byte = value & lower_seven_bits_mask;
      value >>= CHAR_BIT - 1;
      if (value != 0) {
        byte |= leading_bit_mask;
      }
      output.append(&byte, 1);
    } while (value != 0);
  }

  return output;
}

class DecodeException : std::exception {};

template <typename int_type>
int_type decode(Stream* stream) {
  constexpr unsigned lower_seven_bits_mask = 0x7FU;
  constexpr unsigned continuation_bit_mask = 0x80U;
  static_assert(sizeof(int_type) == sizeof(uint32_t) ||
                    sizeof(int_type) == sizeof(uint64_t),
                "Only 32bit and 64bit integers are supported");
  constexpr unsigned size = sizeof(int_type) * CHAR_BIT;
  int_type result = 0;
  unsigned shift = 0;
  uint8_t byte = continuation_bit_mask;
  while ((shift < size) && (byte & continuation_bit_mask)) {
    byte = stream->ReadByte();
    result |= int_type(byte & lower_seven_bits_mask) << shift;
    shift += CHAR_BIT - 1;
  }

  if (byte & continuation_bit_mask) [[unlikely]] {
    // Overflow!
    throw DecodeException();
  }

  if constexpr (std::is_signed_v<int_type>) {
    constexpr unsigned size = sizeof(int_type) * CHAR_BIT;
    constexpr unsigned sign_bit_mask = 0x40U;
    if ((shift < size) && (byte & sign_bit_mask)) {
      // sign extend
      result |= ~int_type(0) << shift;
    }
  }

  return result;
}

}  // namespace wasmcc::leb128
