#pragma once
#include <cstdint>
#include <string>


namespace CodeBase
{
   class ErrorCode
   {
   public:
      enum 
      {
         OK = 0,
      };

      constexpr static ErrorCode MakeError(std::uint64_t code,const char* detail = nullptr);
      static ErrorCode MakeErrorWithFrame(std::uint64_t code,const char* detail = nullptr);
      std::string to_string()const;

     
      ErrorCode(const ErrorCode&) = default;
      ErrorCode(ErrorCode&&) = default;
      ErrorCode& operator=(const ErrorCode&) = default;
      ErrorCode& operator=(ErrorCode&&) = default;
      ~ErrorCode() = default;
      bool operator==(const ErrorCode& rhs)const = default;
   private:
      constexpr ErrorCode(std::uint64_t code,const char* detail = nullptr, std::string_view strview = "");
      uint64_t code = 0;
      const char* detailMsg = nullptr;
      std::string frame;
   };
   
}