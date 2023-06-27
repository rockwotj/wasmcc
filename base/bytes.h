#include <cstdint>
#include <span>
#include <string>

#pragma once

namespace wasmcc {

using bytes = std::basic_string<uint8_t>;
using bytes_view = std::span<uint8_t>;

bool operator==(bytes, bytes_view);

}  // namespace wasmcc
