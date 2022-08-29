#include "tokenizer.hpp"
#include <tuple>
namespace CodeBase
{
    std::tuple<size_t,size_t,ErrorCode> TokenizerStream::nextToken()
    {
        auto begin = pos;
        for(;begin < str.size() && is_whitespace(str[begin]);++begin);
        if(begin == str.size())
            return  {str.size(),0,ErrorCode::eof()};
        auto end = begin;
        if(is_alphabat(str[end])){
            for(;end < str.size() && is_identifier(str[end])  ; ++end){}
            return {begin,end,ErrorCode::OK()};
        }else if(end+1 < str.size() && str[begin+1] == '=' &&
            (str[end] == '<' || str[end] == '>' || str[end] == '!' || str[end] == '=')){
            return {begin,begin+2,  ErrorCode::OK()};
        }else if(is_number(str[end])){
            bool pointFlag = false;
            for(;end < str.size(); ++end)
            {
                if(is_number(str[end]))
                    continue;
                else if(str[end] == '.')
                {
                    if(pointFlag)
                        return {begin,end,ErrorCode::ParseError()};
                    pointFlag = true;
                    continue;
                }else if(is_identifier(str[end])){
                    return {begin,end,ErrorCode::ParseError()};
                }else{
                    break;
                }
            }
            return {begin,end,ErrorCode::OK()};
        }else if (end +1 < str.size() && 
            ((str[end] == '|' && str[end+1] == '|') || (str[end] == '&' && str[end+1] == '&'))){
            return {begin,end+2,ErrorCode::OK()};
        }else if(str[end] == '"' || str[end] == '\''){
            auto quote = str[end];
            for(;end < str.size(); ++end)
            {
                if(str[end] == quote)
                    break;
                else if(str[end] == '\\')
                {
                    ++end;
                    if(end == str.size())
                        return {begin,end,ErrorCode::ParseError()};
                    continue;
                }
            }
            if(end == str.size())
                return {begin,end,ErrorCode::ParseError()};
            return {begin,end+1,ErrorCode::OK()};

        } else{
            return {begin,begin+1,ErrorCode::OK()};
        }
    }
    std::pair<std::string_view,ErrorCode> TokenizerStream::getToken(){
        auto [begin,end,err] = nextToken();
        if(err == ErrorCode::OK()){
            pos = end;
            return {str.substr(begin,end-begin),err};
        }else{
            return {str.substr(begin,end-begin),err};
        }
    }
    std::pair<std::string_view,ErrorCode> TokenizerStream::previewToken(){
        auto [begin,end,err] = nextToken();
        return {str.substr(begin,end-begin),err};
    }
    bool TokenizerStream::EatToken(const std::string_view token){
        auto [preview,err] = previewToken();
        if(err != ErrorCode::OK() || preview != token){
            return false;
        }
        getToken();
        return true;
    }
    bool TokenizerStream::EatTokenWithoutCaseSensitive(const std::string_view token)
    {
        auto [preview,err] = previewToken();
        if(err != ErrorCode::OK() || compareWithoutCaseSensitive(preview,token)){
            return false;
        }
        getToken();
        return true;
    }
    bool TokenizerStream::eof()const{
        return pos == str.size();
    }
    void TokenizerStream::reset(){
        pos = 0;
    }
}