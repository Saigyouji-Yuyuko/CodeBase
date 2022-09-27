#include "utils/error.hpp"
#include <boost/stacktrace.hpp>
#include <sstream>
namespace CodeBase
{
    constexpr ErrorCode::ErrorCode(std::uint64_t code,const char* detail , std::string_view strview):
                    code(code),detailMsg(detail),frame(strview){}

    constexpr ErrorCode ErrorCode::MakeError(std::uint64_t code,const char* detail )
    {
        return ErrorCode{code,detail};
    }
    ErrorCode ErrorCode::MakeErrorWithFrame(std::uint64_t code,const char* detail)
    {
        auto tmp = boost::stacktrace::stacktrace(1,static_cast<std::size_t>(-1));
        return ErrorCode{code,detail,boost::stacktrace::to_string(tmp)};
    }
    std::string ErrorCode::to_string()const{
        std::stringstream ss;
        ss<<"[error code: "<<this->code;
        if(this->detailMsg != nullptr){
            ss<<"; detail: "<<this->detailMsg;
        }
        if(!this->frame.empty()){
            ss<<"; frame: "<<this->frame;
        }
        ss<<"].";
        return ss.str();
    }
}