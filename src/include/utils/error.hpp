#pragma once
#include <cstdint>
#include <string>
#include <string_view>

namespace CodeBase {
#define ERROR_CODE(XX)                                                                                                 \
    XX(InvalidArgs, "InvalidArgs", "Invalid arguments")                                                                \
    XX(Fail, "Fail", "Fail")                                                                                           \
    XX(Eof, "Eof", "End of file")                                                                                      \
    XX(NotSupport, "NotSupport", "Feature Not support yet")                                                            \
    XX(OutOfRange, "OutOfRange", "Out of range")

enum {
    ErrorNoSuccess = 0,
#define XX(code, str1, str2) ErrorNo##code,
    ERROR_CODE(XX)
#undef XX
            TotalError
};

constexpr const char *shortName[] = {"Success",
#define XX(code, str1, str2) str1,
                                     ERROR_CODE(XX)
#undef XX
};

constexpr const char *detailStr[] = {"Success",
#define XX(code, str1, str2) str2,
                                     ERROR_CODE(XX)
#undef XX
};

class ErrorCode {
public:
    constexpr explicit ErrorCode() = default;
    constexpr explicit ErrorCode(std::uint64_t code) : code(code) {}
    ~ErrorCode() = default;
    constexpr std::string_view short_name() const {
        return shortName[code];
    }
    constexpr std::string_view detail() const {
        return detailStr[code];
    }
    constexpr bool operator==(const ErrorCode &other) const {
        return code == other.code;
    }

private:
    uint64_t code = 0;
};

namespace Error {
constexpr ErrorCode Success = ErrorCode(uint64_t(0));
#define XX(code, str1, str2) constexpr ErrorCode code = ErrorCode(ErrorNo##code);
ERROR_CODE(XX)
#undef XX
#undef ERROR_CODE
}// namespace Error

}// namespace CodeBase