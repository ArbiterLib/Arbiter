file(GLOB arbiter_sources *.cpp)
add_library("${ARBITER_TARGET}" STATIC ${arbiter_sources})
target_include_directories("${ARBITER_TARGET}"
  PUBLIC "${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/src")

