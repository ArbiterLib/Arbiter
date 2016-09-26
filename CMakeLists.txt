cmake_minimum_required(VERSION 3.6.2)

project(Arbiter)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF) # -std=c++14 (otherwise cmake uses -std=gnu++14)

string(APPEND CMAKE_C_FLAGS " -pedantic -Wall -Wextra")
string(APPEND CMAKE_CXX_FLAGS " -pedantic -Wall -Wextra")

set(ARBITER_TARGET Arbiter)
add_subdirectory(src)
add_subdirectory(examples/library_folders)

include(CTest)
if(BUILD_TESTING)
  add_subdirectory("${CMAKE_SOURCE_DIR}/external/googletest/googletest")
  add_subdirectory(test)
endif()

