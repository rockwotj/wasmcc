#include "core/coro.h"

#include <gtest/gtest.h>

#include <exception>

namespace wasmcc {

future<int> f() { co_return 4; }

future<int> g() {
    int a = co_await f();
    int b = co_await f();
    co_return a + b;
}

future<int> h() {
    auto a = co_await f();
    auto b = co_await f();
    auto c = co_await g();
    co_return a + b - c;
}

TEST(CoroutineTests, SmokeTest) {
    wasmcc::future<int> f = wasmcc::h();
    EXPECT_EQ(f.get(), 0);
}

} // namespace wasmcc
