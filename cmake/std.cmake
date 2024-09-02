# Compile std as a module (LLVM only !)

# extract the compiler directory
get_filename_component(COMPILER_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
set(STD_BASE_DIR ${COMPILER_DIR}/../share/libc++/v1)

# compile the std library
add_library(std-cxx-modules STATIC)
target_sources(std-cxx-modules
  PUBLIC
    FILE_SET moduleStd
    TYPE CXX_MODULES
    BASE_DIRS ${STD_BASE_DIR}
    FILES
      ${STD_BASE_DIR}/std.cppm
      ${STD_BASE_DIR}/std.compat.cppm)
target_compile_options(std-cxx-modules
  PRIVATE
    -Wno-reserved-module-identifier
    -Wno-reserved-user-defined-literal)
target_link_libraries(${PROJECT_NAME} std-cxx-modules)