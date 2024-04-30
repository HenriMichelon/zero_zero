cmake_minimum_required(VERSION 3.22)
cmake_policy(SET CMP0002 NEW)

project(ZeroZero)

# Engine sources directory
set(Z0_ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
# GLSL shaders sources directory
set(Z0_SHADERS_DIR ${Z0_ENGINE_DIR}/shaders)
# SPIR-V compiled shaders directory
set(Z0_SHADERS_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
make_directory(${Z0_SHADERS_BUILD_DIR})

include(FetchContent)
if(EXISTS .env.cmake)
	include(.env.cmake)
endif ()
include(cmake/shaders.cmake)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_COMPILE_WARNING_AS_ERROR OFF) # disabled due to fastgltf and VMA
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_definitions(VULKAN_STATS) # print some stats at the end of the execution

# compile GLSL shaders
file(GLOB_RECURSE Z0_GLSL_SOURCE_FILES
        "${Z0_SHADERS_DIR}/*.frag"
        "${Z0_SHADERS_DIR}/*.vert"
)
add_shaders(${PROJECT_NAME}_shaders ${Z0_GLSL_SOURCE_FILES})

# compile engine + examples
add_executable(${PROJECT_NAME}
        examples/exemple_1.cpp
)

add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_shaders)
target_include_directories(${PROJECT_NAME} PUBLIC ${Z0_ENGINE_DIR}/include)

include(cmake/libraries.cmake)
