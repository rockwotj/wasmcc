#include "core/coro.h"

namespace wasmcc::co {
future<> maybe_yield() { co_return; }
} // namespace wasmcc::co
