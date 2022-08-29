project(codebase)
cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)
#include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
#conan_basic_setup()
set(CMAKE_EXPORT_COMPILE_COMMANDS on)
#add_executable(main main.cpp sqlImpl.cpp)
#target_link_libraries(main ${CONAN_LIBS})

enable_testing()
add_executable(
  hello_test
  tokenize_test.cpp
  tokenizer.cpp
)
 target_link_libraries(
   hello_test
   gtest
   gtest_main
 )

include(GoogleTest)
gtest_discover_tests(hello_test)