#pragma once
#include <cstdint>
namespace CodeBase
{
   class ErrorCode
   {
   public:
      ErrorCode(const ErrorCode&) = default;
      ErrorCode(ErrorCode&&) = default;
      ErrorCode& operator=(const ErrorCode&) = default;
      ErrorCode& operator=(ErrorCode&&) = default;
      ~ErrorCode() = default;
      static constexpr ErrorCode OK() { return ErrorCode(0); }
      static constexpr ErrorCode NotSupport() { return ErrorCode(1); }
      static constexpr ErrorCode SyntaxError() { return ErrorCode(2); }
      static constexpr ErrorCode NotFound() { return ErrorCode(3); }
      static constexpr ErrorCode NotImplemented() { return ErrorCode(4); }
      static constexpr ErrorCode ParseError() { return ErrorCode(5); }
      static constexpr ErrorCode FiledError() { return ErrorCode(6); }
      static constexpr ErrorCode eof() { return ErrorCode(7); }
      bool operator==( const ErrorCode& rhs)const = default;
   private:
      constexpr ErrorCode(std::uint64_t code) : code(code) {}
      uint64_t code = 0;
      const char* detailMsg = nullptr;
      uint64_t lineNumber = 0;
      const char* fileName = nullptr;
   };
   
}