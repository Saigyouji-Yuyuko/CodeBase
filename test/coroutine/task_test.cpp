#include "gtest/gtest.h"

#include "coroutine/Task.hpp"
#include <memory>
#include <string_view>
using namespace CodeBase;
using namespace testing;

Task<int> GetOne() {
    co_return 1;
}

Task<int> GetOne2() {
    co_return co_await []() -> Task<int> { co_return 1; }();
}


Task<int> GetTWO() {
    auto a = co_await GetOne();
    auto b = co_await GetOne2();
    co_return (a + b);
}

TEST(TaskTest, Test1) {
    auto t = GetOne();
    EXPECT_EQ(t.sync(), 1);
    t = GetTWO();
    EXPECT_EQ(t.sync(), 2);

    auto tmp = []() -> Task<int> { co_return 1; };
    EXPECT_EQ(tmp().sync(), 1);
}