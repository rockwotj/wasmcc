#pragma once

#include <cstddef>
#include <cstdint>

namespace wasmcc {

/**
 * Align value up to the nearest 'align' bytes.
 */
uint32_t AlignUp(uint32_t, uint32_t alignment);
uint64_t AlignUp(uint64_t, uint64_t alignment);
/**
 * Align value down to the nearest 'align' bytes.
 */
uint32_t AlignDown(uint32_t, uint32_t alignment);
uint64_t AlignDown(uint64_t, uint64_t alignment);

}  // namespace wasmcc
