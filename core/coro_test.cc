#include "core/coro.h"

#include <gtest/gtest.h>

#include <exception>

namespace wasmcc {

co::future<int> f() { co_return 4; }

co::future<int> g() {
    int a = co_await f();
    int b = co_await f();
    co_return a + b;
}

co::future<int> h() {
    auto a = co_await f();
    auto b = co_await f();
    auto c = co_await g();
    co_return a + b - c;
}

TEST(CoroutineTests, SmokeTest) {
    co::future<int> f = h();
    EXPECT_EQ(f.get(), 0);
}

TEST(CoroutineTests, MaybeYield) { co::maybe_yield().get(); }

} // namespace wasmcc
