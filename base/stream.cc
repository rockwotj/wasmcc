#include "base/stream.h"

#include <algorithm>
#include <stdexcept>

namespace wasmcc {

ByteStream::ByteStream(bytes b) : _buffer(std::move(b)), _position(0) {}

uint8_t ByteStream::ReadByte() {
  if (!HasRemaining()) [[unlikely]] {
    throw EndOfStreamException();
  }
  return _buffer[_position++];
}
uint8_t ByteStream::PeekByte() {
  if (!HasRemaining()) [[unlikely]] {
    throw EndOfStreamException();
  }
  return _buffer[_position];
}
bytes ByteStream::ReadBytes(size_t n) {
  auto remainder = _buffer.size() - _position;
  if (remainder < n) [[unlikely]] {
    throw EndOfStreamException();
  }
  auto start = _buffer.begin() + int32_t(_position);
  auto end = start + int32_t(n);
  bytes b(start, end);
  _position += n;
  return b;
}
void ByteStream::Skip(size_t n) {
  auto remainder = _buffer.size() - _position;
  if (remainder < n) [[unlikely]] {
    throw EndOfStreamException();
  }
  _position += n;
}
bool ByteStream::HasRemaining() { return _position < _buffer.size(); }
size_t ByteStream::BytesConsumed() { return _position; }

}  // namespace wasmcc
