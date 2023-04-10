include_directories(../..)
add_executable(
  tokenize_test
  tokenize_test.cpp
)
target_link_libraries(
    tokenize_test
    PUBLIC
    gtest
    gtest_main
    sqlFrontend
)
 
include(GoogleTest)
gtest_discover_tests(tokenize_test)