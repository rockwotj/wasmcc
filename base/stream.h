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
   * Read a single byte from the stream without advancing the stream.
   *
   * Implementations should throw EndOfStreamException if there is no data
   * left.
   */
  virtual uint8_t PeekByte() = 0;

  /**
   * Write bytes into the bytes view, return the number of bytes written.
   *
   * Implementations should throw EndOfStreamException if there is not
   * enough data left to read.
   */
  virtual bytes ReadBytes(size_t) = 0;

  /**
   * A more efficent version of ReadBytes that doesn't allocate.
   */
  virtual void Skip(size_t) = 0;

  /** If there is data left on the stream. */
  virtual bool HasRemaining() = 0;
  /** The number of bytes that have been consumed on the stream. */
  virtual size_t BytesConsumed() = 0;
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
  uint8_t PeekByte() override;
  bytes ReadBytes(size_t) override;
  void Skip(size_t) override;
  bool HasRemaining() override;
  size_t BytesConsumed() override;

 private:
  bytes _buffer;
  size_t _position;
};

}  // namespace wasmcc
