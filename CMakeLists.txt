cmake_minimum_required(VERSION 3.6.2)

project(Arbiter)

string(APPEND CMAKE_C_FLAGS " -std=c99 -pedantic -Wall -Wextra")
string(APPEND CMAKE_CXX_FLAGS " -std=c++14 -pedantic -Wall -Wextra")

set(ARBITER_TARGET Arbiter)
add_subdirectory(src)
add_subdirectory(examples/library_folders)

include(CTest)
if(BUILD_TESTING)
  add_subdirectory("${CMAKE_SOURCE_DIR}/external/googletest/googletest")
  add_subdirectory(test)
endif()

