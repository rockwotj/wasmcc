
#include "runtime/thread/thread.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <iostream>

namespace wasmcc::runtime {

constexpr size_t kStackSize = 8L * 1024;

TEST(VMThread, Works) {
  bool running = true;
  int invoke_count = 0;
  auto thread = VMThread::Create(
      [&invoke_count, &running] {
        while (running) {
          ++invoke_count;
          VMThread::Yield();
        }
      },
      kStackSize);
  for (int i = 0; i < 4; ++i) {
    EXPECT_EQ(invoke_count, i);
    thread->Resume();
  }
  EXPECT_EQ(invoke_count, 4);
  running = false;
  thread->Resume();
  EXPECT_EQ(invoke_count, 4);
}

TEST(VMThread, CanBeStartedAfterStop) {
  int invoke_count = 0;
  auto thread =
      VMThread::Create([&invoke_count] { ++invoke_count; }, kStackSize);
  for (int i = 0; i < 4; ++i) {
    EXPECT_EQ(invoke_count, i);
    thread->Resume();
  }
  EXPECT_EQ(invoke_count, 4);
}

}  // namespace wasmcc::runtime
