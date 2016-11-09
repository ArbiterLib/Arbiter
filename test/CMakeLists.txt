# Adds a googletest-based test named for the given source file,
# linked against Aribter.
function(add_googletest test_source)
  get_filename_component(test_name "${test_source}" NAME_WE)
  add_executable("${test_name}" TestValue.cpp "${test_name}.cpp")
  target_link_libraries("${test_name}"
    PRIVATE "${ARBITER_TARGET}" gtest gtest_main)
  target_include_directories("${test_name}"
    SYSTEM
    PRIVATE
      "${CMAKE_SOURCE_DIR}/external/googletest/googletest/include"
      "${PROJECT_SOURCE_DIR}/src")
  add_test("${test_name}" "${test_name}" COMMAND "${test_name}")
endfunction()

set(test_sources
  CarthageGraphTest.cpp
  RequirementTest.cpp
  ResolverTest.cpp
  VersionTest.cpp)
foreach(test_source ${test_sources})
  add_googletest("${test_source}")
endforeach()

