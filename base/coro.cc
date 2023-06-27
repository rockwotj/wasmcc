#include "base/coro.h"

namespace wasmcc::co {
future<> MaybeYield() { co_return; }
}  // namespace wasmcc::co
