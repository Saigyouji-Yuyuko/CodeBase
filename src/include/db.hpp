#pragma once

#include "utils/error.hpp"
#include <cstdint>
#include <memory>
#include <string>
namespace CodeBase {
class TableView;

class DBInterface {
    using DBType = uint64_t;
    static ErrorCode  Format(std::string path, std::unique_ptr<DBInterface> &db, DBType t);
    virtual ErrorCode Exec(const std::string statement, TableView &result) = 0;
    virtual ~DBInterface() = default;
};
}// namespace CodeBase