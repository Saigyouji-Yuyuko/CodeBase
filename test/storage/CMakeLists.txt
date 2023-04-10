include_directories(../..)
add_executable(
  storage_test
  storage_test.cpp
)
target_link_libraries(
    storage_test
    PUBLIC
    gtest
    gtest_main
    Storage
)
 
include(GoogleTest)
gtest_discover_tests(storage_test)