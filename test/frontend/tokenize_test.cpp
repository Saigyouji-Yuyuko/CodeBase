#include "gtest/gtest.h"

#include "tokenizer.hpp"
#include <string_view>
using namespace CodeBase;
using namespace testing;

TEST(TokenizerTest, TestGetToken) {
    TokenizerStream stream("a");
    auto [token, error] = stream.getToken();
    EXPECT_EQ(error, Error::Success);
    EXPECT_EQ(token, std::string_view("a"));
    EXPECT_EQ(stream.getToken().second, Error::Eof);
}

TEST(TokenizerTest, TestGetToken2) {
    TokenizerStream stream("select * from table");
    std::string     ans[] = {"select", "*", "from", "table"};
    for (auto i = 0;; ++i) {
        auto [token, error] = stream.getToken();
        if (error == Error::Eof)
            break;
        EXPECT_EQ(error, Error::Success);
        EXPECT_EQ(token, ans[i]);
        std::cout << token << std::endl;
    }
}

TEST(TokenizerTest, TestGetToken3) {
    TokenizerStream stream("select a.* , (a+b*c*+235.1233) as ttt from table");
    std::string     ans[] = {"select", "a", ".", "*",        ",", "(",  "a",   "+",    "b",    "*",
                             "c",      "*", "+", "235.1233", ")", "as", "ttt", "from", "table"};
    for (auto i = 0;; ++i) {
        auto [token, error] = stream.getToken();
        if (error == Error::Eof)
            break;
        EXPECT_EQ(error, Error::Success);
        EXPECT_EQ(token, ans[i]) << i << token << " " << ans[i] << std::endl;
        std::cout << token << std::endl;
    }
}