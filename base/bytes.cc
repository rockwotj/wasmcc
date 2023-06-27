#include "base/bytes.h"

#include <algorithm>

namespace wasmcc {
bool operator==(bytes lhs, bytes_view rhs) {
  return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
}  // namespace wasmcc
