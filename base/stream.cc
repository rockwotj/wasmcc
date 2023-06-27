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
size_t ByteStream::ReadBytes(bytes_view out) {
  size_t amt = std::min(out.size(), _buffer.size() - _position);
  std::copy_n(&_buffer[_position], amt, out.begin());
  _position += amt;
  return amt;
}
size_t ByteStream::Skip(size_t n) {
  size_t amt = std::min(n, _buffer.size() - _position);
  _position += amt;
  return amt;
}
bool ByteStream::HasRemaining() { return _position < _buffer.size(); }
size_t ByteStream::BytesConsumed() { return _position; }

}  // namespace wasmcc
