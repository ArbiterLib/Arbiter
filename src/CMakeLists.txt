add_library("${ARBITER_TARGET}" STATIC
  Dependency.cpp
  Exception.cpp
  Graph.cpp
  Instantiation.cpp
  Project.cpp
  Requirement.cpp
  Resolver.cpp
  Stats.cpp
  ToString.cpp
  Types.cpp
  Value.cpp
  Version.cpp)
target_include_directories("${ARBITER_TARGET}"
  PUBLIC "${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/src")

