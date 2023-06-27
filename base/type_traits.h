#include <type_traits>

namespace wasmcc {
template <typename>
struct DependantFalse : std::false_type {};

template <typename... Args>
concept EmptyPack = sizeof...(Args) == 0;

}  // namespace wasmcc
