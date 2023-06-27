#include "base/coro.h"

namespace wasmcc::co {
Future<> MaybeYield() { co_return; }
}  // namespace wasmcc::co
