set(LIBRARY_FOLDERS_TARGET library_folders)

add_executable("${LIBRARY_FOLDERS_TARGET}" EXCLUDE_FROM_ALL
  asprintf.c
  dependencies.c
  main.c
  strerror.c
  string_value.c)
target_link_libraries("${LIBRARY_FOLDERS_TARGET}" "${ARBITER_TARGET}")
target_include_directories("${LIBRARY_FOLDERS_TARGET}"
  PUBLIC "${PROJECT_SOURCE_DIR}/include")

