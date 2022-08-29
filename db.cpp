#include "db.hpp"

namespace CodeBase
{
    ErrorCode DBInterface::Format(std::string path,std::unique_ptr<DBInterface>& db,DBInterface::DBType t)
    {
        return ErrorCode::NotSupport();
    }
    ErrorCode DBInterface::Exec(const std::string statement,TableView& result)
    {
        return ErrorCode::NotSupport();
    }
}