#include "core/instruction.h"

#include "absl/strings/str_format.h"

namespace wasmcc {

LabelId MakeLabelId(std::string_view name, int32_t id) {
  return LabelId(absl::StrFormat("%s_%d", name, id));
}

}  // namespace wasmcc
