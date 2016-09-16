project(Arbiter C CXX)

cmake_minimum_required(VERSION 3.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pedantic -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pedantic -Wall -Wextra")

set(ARBITER_TARGET Arbiter)
add_subdirectory(src)
add_subdirectory(examples/library_folders)

include(CTest)
if(BUILD_TESTING)
  add_subdirectory("${CMAKE_SOURCE_DIR}/external/googletest/googletest")
  add_subdirectory(test)
endif()

