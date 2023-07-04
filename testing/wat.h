#pragma once

#include <string_view>

#include "base/bytes.h"

namespace wasmcc {

/**
 * Convert a WAT file into it's binary form.
 *
 * https://developer.mozilla.org/en-US/docs/WebAssembly/Understanding_the_text_format
 */
bytes Wat2Wasm(std::string_view);

}  // namespace wasmcc
