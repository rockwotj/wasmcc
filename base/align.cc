#include "base/align.h"

namespace wasmcc {
namespace {
template <typename T>
T AlignDown(T v, T alignment) {
  return v & ~(alignment - 1);
}
template <typename T>
T AlignUp(T v, T alignment) {
  return AlignDown<T>(v + alignment - 1, alignment);
}
}  // namespace

uint32_t AlignUp(uint32_t v, uint32_t alignment) {
  return AlignUp<uint32_t>(v, alignment);
}
uint64_t AlignUp(uint64_t v, uint64_t alignment) {
  return AlignUp<uint64_t>(v, alignment);
}
uint32_t AlignDown(uint32_t v, uint32_t alignment) {
  return AlignDown<uint32_t>(v, alignment);
}
uint64_t AlignDown(uint64_t v, uint64_t alignment) {
  return AlignDown<uint64_t>(v, alignment);
}
}  // namespace wasmcc
