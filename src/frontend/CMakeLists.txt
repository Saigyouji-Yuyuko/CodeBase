include_directories(../frontend)
add_library(
    sqlFrontend
    sqlparse.cpp
    tokenizer.hpp
)

target_include_directories(sqlFrontend PUBLIC .)
