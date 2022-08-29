#include <iostream>
#include <cstdint>
#include <string>
#include <chrono>

namespace codebase
{
    class SuperBlock
    {
        std::uint64_t magic;
        std::uint64_t version;
        std::uint64_t blockSize;
        std::uint64_t formatTime;
        
    };
    class TableView
    {

    };
    class ErrorCode
    {

    };
    class DB
    {
        std::string RootPath;
        static void Format(std::string path);
        ErrorCode Exec(const std::string statement,TableView& result);
    };
}

int main()
{

    return 0;
}