#pragma once

#include <string_view>

#include "base/bytes.h"

namespace wasmcc {

bytes Wat2Wasm(std::string_view);

}
