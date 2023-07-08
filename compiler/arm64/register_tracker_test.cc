#include "compiler/arm64/register_tracker.h"

#include <gtest/gtest-matchers.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <functional>
#include <iterator>
#include <optional>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "gmock/gmock.h"

template <>
struct std::hash<asmjit::a64::Gp> {
  std::size_t operator()(asmjit::a64::Gp const& reg) const noexcept {
    std::size_t h1 = std::hash<uint32_t>{}(reg.id());
    std::size_t h2 = std::hash<uint32_t>{}(reg.signature().bits());
    return h1 ^ (h2 << 1U);
  }
};

namespace wasmcc::arm64 {

TEST(RegisterTracker, AllocatesRegisters) {
  RegisterTracker rt;
  auto r1 = rt.TakeUnusedRegister();
  EXPECT_NE(r1, std::nullopt);
  auto r2 = rt.TakeUnusedRegister();
  EXPECT_NE(r2, std::nullopt);
  EXPECT_NE(r1, r2);
}

TEST(RegisterTracker, CanAllocateAllRegisters) {
  RegisterTracker rt;
  std::vector<asmjit::a64::Gp> registers;
  while (true) {
    auto reg = rt.TakeUnusedRegister();
    if (!reg.has_value()) {
      break;
    }
    registers.push_back(*reg);
  }
  absl::flat_hash_set<asmjit::a64::Gp> unique(registers.begin(),
                                              registers.end());
  EXPECT_THAT(registers, testing::UnorderedElementsAreArray(unique));
}
}  // namespace wasmcc::arm64
