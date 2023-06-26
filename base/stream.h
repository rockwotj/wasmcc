#pragma once

#include <cstdint>

#include "base/bytes.h"

namespace wasmcc {

/** There are no more bytes left in the stream. */
class EndOfStreamException : public std::exception {};

/**
 * A abstract class for a stream of data.
 */
class Stream {
 public:
  Stream() = default;
  Stream(const Stream&) = delete;
  Stream& operator=(const Stream&) = delete;
  Stream(Stream&&) = default;
  Stream& operator=(Stream&&) = default;
  virtual ~Stream() = default;

  /**
   * Read a single byte from the stream.
   *
   * Implementations should throw EndOfStreamException if there is no data
   * left.
   */
  virtual uint8_t ReadByte() = 0;

  /**
   * Write bytes into the bytes view, return the number of bytes written.
   *
   * This may read 0 bytes if there is no data left in the stream.
   */
  virtual size_t ReadBytes(bytes_view) = 0;

  /** If there is data left on the stream. */
  virtual bool HasRemaining() = 0;
};

/**
 * A stream over a contiguous range of bytes.
 */
class ByteStream : public Stream {
 public:
  explicit ByteStream(bytes);
  ByteStream(const ByteStream&) = delete;
  ByteStream& operator=(const ByteStream&) = delete;
  ByteStream(ByteStream&&) = default;
  ByteStream& operator=(ByteStream&&) = default;
  ~ByteStream() override = default;

  uint8_t ReadByte() override;
  size_t ReadBytes(bytes_view) override;
  bool HasRemaining() override;

 private:
  bytes _buffer;
  size_t _position;
};

}  // namespace wasmcc
