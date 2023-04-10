include_directories(../..)
add_executable(
  coroutine_test
  task_test.cpp
) 
target_link_libraries(
  coroutine_test
    PUBLIC
    gtest
    gtest_main
)
 
include(GoogleTest)
gtest_discover_tests(coroutine_test)