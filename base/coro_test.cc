#include "base/coro.h"

#include <gtest/gtest.h>

#include <exception>

namespace wasmcc {

struct MoveOnly {
  explicit MoveOnly(int f) : foo(f){};
  MoveOnly(const MoveOnly&) = delete;
  MoveOnly& operator=(const MoveOnly&) = delete;
  MoveOnly(MoveOnly&&) = default;
  MoveOnly& operator=(MoveOnly&&) = default;
  ~MoveOnly() = default;

  int foo;
};

co::Future<MoveOnly> m() { co_return MoveOnly(1); }
co::Future<MoveOnly> n() {
  auto a = co_await m();
  auto b = co_await m();
  co_return MoveOnly(a.foo + b.foo);
}

co::Future<int> f() { co_return 4; }

co::Future<int> g() {
  int a = co_await f();
  int b = co_await f();
  co_return a + b;
}

co::Future<int> h() {
  auto a = co_await f();
  auto b = co_await f();
  auto c = co_await g();
  co_return a + b - c;
}

TEST(CoroutineTests, SmokeTest) {
  co::Future<int> f = h();
  EXPECT_EQ(f.get(), 0);
}

TEST(CoroutineTests, MaybeYield) { co::MaybeYield().get(); }

TEST(CoroutineTests, MoveOnly) {
  co::Future<MoveOnly> f = n();
  EXPECT_EQ(f.get().foo, 2);
}

}  // namespace wasmcc
