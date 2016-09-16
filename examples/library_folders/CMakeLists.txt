set(LIBRARY_FOLDERS_TARGET library_folders)

file(GLOB library_folders_sources *.c)
add_executable("${LIBRARY_FOLDERS_TARGET}" ${library_folders_sources})
target_link_libraries("${LIBRARY_FOLDERS_TARGET}" "${ARBITER_TARGET}")
target_include_directories("${LIBRARY_FOLDERS_TARGET}"
  PUBLIC "${PROJECT_SOURCE_DIR}/include")

