#pragma once
#include "utils/error.hpp"
#include <string>
namespace CodeBase {
    static inline bool is_alphabat(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }
    static inline bool is_number(char c){
        return c >= '0' && c <= '9';
    }
   
    static inline bool is_identifier(char c)
    {
        return is_alphabat(c) || is_number(c) || c == '_';
    }
    static inline bool is_whitespace(char c)
    {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    }

    static inline bool low2upperCompare(char a,char b){
        return a>='a' && a<= 'z' && b>='A' && b<= 'Z' && ((a+('A'-'a')) == b);
    }
    static inline bool compareWithoutCaseSensitive(char a,char b){
        return a== b || low2upperCompare(a,b) || low2upperCompare(b,a);
    }
    static inline bool compareWithoutCaseSensitive(std::string_view a,std::string_view b){
        if(a.size()!= b.size()){
            return false;
        }
        for(auto i = 0;i!=a.size();++i){
            if(!compareWithoutCaseSensitive(a[i],b[i])){
                return false;
            }
        }
        return true;
    }

    class TokenizerStream
    {
    public:
        TokenizerStream(std::string_view str) : str(str) {}
        std::pair<std::string_view,ErrorCode> getToken();
        std::pair<std::string_view,ErrorCode> previewToken();
        bool EatToken(const std::string_view token);
        bool EatTokenWithoutCaseSensitive(const std::string_view token);
        bool eof()const;
        void reset();
    private:
        std::tuple<size_t,size_t,ErrorCode> nextToken();
        std::string_view str;
        std::size_t pos = 0;
    };
}
