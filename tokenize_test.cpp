#include "gtest/gtest.h"

#include <string_view>
#include "tokenizer.hpp"
using namespace CodeBase;
using namespace testing;

TEST(TokenizerTest, TestGetToken)
{
    TokenizerStream stream("a");
    auto [token , error] = stream.getToken();
    EXPECT_EQ(error, ErrorCode::OK());
    EXPECT_EQ(token, std::string_view("a"));
    EXPECT_EQ(stream.getToken().second, ErrorCode::eof());
}

TEST(TokenizerTest, TestGetToken2)
{
    TokenizerStream stream("select * from table");
    std::string ans[] = {"select", "*", "from", "table"};
    for(auto i =0;;++i){
        auto [token , error] = stream.getToken();
        if(error == ErrorCode::eof())
            break;
        EXPECT_EQ(error, ErrorCode::OK());
        EXPECT_EQ(token,ans[i]);
        std::cout<<token<<std::endl;
    }
}

TEST(TokenizerTest, TestGetToken3)
{
    TokenizerStream stream("select a.* , (a+b*c*+235.1233) as ttt from table");
    std::string ans[] = {"select", "a",".","*", ",", "(","a","+","b","*","c","*","+","235.1233",")","as","ttt","from","table"};
    for(auto i =0;;++i){
        auto [token , error] = stream.getToken();
        if(error == ErrorCode::eof())
            break;
        EXPECT_EQ(error, ErrorCode::OK());
        EXPECT_EQ(token,ans[i])<<i<<token<<" "<<ans[i]<<std::endl;
        std::cout<<token<<std::endl;
    }
}