include_directories(../frontend)
add_library(
    sqlFrontend
    sqlparse.cpp
    tokenizer.cpp
)

target_include_directories(sqlFrontend PUBLIC .)
