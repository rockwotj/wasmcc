
#include "runtime/thread/thread.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <iostream>
#include <memory>

#include "gtest/gtest.h"

namespace wasmcc::runtime {

constexpr VMThreadConfiguration kConfig = {.stack_size = 8L * 1024,
                                           .enable_guard_pages = true};

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
      kConfig);
  EXPECT_EQ(thread->state(), VMThread::State::kStopped);
  for (int i = 0; i < 4; ++i) {
    EXPECT_EQ(invoke_count, i);
    thread->Resume();
    EXPECT_EQ(thread->state(), VMThread::State::kSuspended);
  }
  EXPECT_EQ(invoke_count, 4);
  EXPECT_EQ(thread->state(), VMThread::State::kSuspended);
  running = false;
  thread->Resume();
  EXPECT_EQ(invoke_count, 4);
  EXPECT_EQ(thread->state(), VMThread::State::kStopped);
}

TEST(VMThread, CanBeStartedAfterStop) {
  int invoke_count = 0;
  auto thread = VMThread::Create([&invoke_count] { ++invoke_count; }, kConfig);
  for (int i = 0; i < 4; ++i) {
    EXPECT_EQ(invoke_count, i);
    thread->Resume();
    EXPECT_EQ(thread->state(), VMThread::State::kStopped);
  }
  EXPECT_EQ(invoke_count, 4);
}

TEST(VMThread, CanBeStoppedWhileSuspended) {
  int value = -1;
  auto thread = VMThread::Create(
      [&value] {
        for (int i = 0; i < 4; ++i) {
          value = i;
          VMThread::Yield();
        }
      },
      kConfig);
  EXPECT_EQ(value, -1);
  EXPECT_EQ(thread->state(), VMThread::State::kStopped);
  thread->Resume();
  EXPECT_EQ(value, 0);
  EXPECT_EQ(thread->state(), VMThread::State::kSuspended);
  thread->Resume();
  EXPECT_EQ(value, 1);
  EXPECT_EQ(thread->state(), VMThread::State::kSuspended);
  thread->Resume();
  EXPECT_EQ(value, 2);
  EXPECT_EQ(thread->state(), VMThread::State::kSuspended);
  thread->Stop();
  EXPECT_EQ(thread->state(), VMThread::State::kStopped);
  thread->Resume();
  EXPECT_EQ(value, 0);
  EXPECT_EQ(thread->state(), VMThread::State::kSuspended);
}

}  // namespace wasmcc::runtime
