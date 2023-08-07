#include <cstdint>
#include <span>
#include <vector>

#pragma once

namespace wasmcc {

using bytes = std::vector<uint8_t>;
using bytes_view = std::span<uint8_t>;

}  // namespace wasmcc
