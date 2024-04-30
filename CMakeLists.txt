cmake_minimum_required(VERSION 3.22)
cmake_policy(SET CMP0002 NEW)

project(ZeroZero)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Engine sources directory
set(Z0_INCLUDE_DIR ${INCLUDE_DIR}/z0)
set(Z0_ENGINE_DIR ${SRC_DIR}/z0)
set(Z0_SHADERS_DIR ${Z0_ENGINE_DIR}/shaders)
# Examples sources directory
set(Z0_EXAMPLES_DIR ${SRC_DIR}/examples)
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
if(MINGW)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
endif()

# compile GLSL shaders
file(GLOB_RECURSE Z0_GLSL_SOURCE_FILES
        "${Z0_SHADERS_DIR}/*.frag"
        "${Z0_SHADERS_DIR}/*.vert"
)
add_shaders(${PROJECT_NAME}_shaders ${Z0_GLSL_SOURCE_FILES})

# compile engine + examples
add_executable(${PROJECT_NAME}
		${INCLUDE_DIR}/vk_mem_alloc.h

		${Z0_INCLUDE_DIR}/constants.h
		${Z0_INCLUDE_DIR}/application_config.h
		${Z0_INCLUDE_DIR}/tools.h
		${Z0_INCLUDE_DIR}/object.h
		${Z0_INCLUDE_DIR}/window.h
		${Z0_INCLUDE_DIR}/application.h
		${Z0_INCLUDE_DIR}/device.h
		${Z0_INCLUDE_DIR}/buffer.h
		${Z0_INCLUDE_DIR}/stats.h

		${Z0_INCLUDE_DIR}/nodes/node.h

		${Z0_INCLUDE_DIR}/resources/resource.h
		${Z0_INCLUDE_DIR}/resources/image.h

		${Z0_ENGINE_DIR}/tools.cpp
		${Z0_ENGINE_DIR}/window.cpp
		${Z0_ENGINE_DIR}/application.cpp
		${Z0_ENGINE_DIR}/device.cpp
		${Z0_ENGINE_DIR}/buffer.cpp
		${Z0_ENGINE_DIR}/stats.cpp

		${Z0_ENGINE_DIR}/nodes/node.cpp

		${Z0_ENGINE_DIR}/resources/resource.cpp
		${Z0_ENGINE_DIR}/resources/image.cpp

		${SRC_DIR}/libraries.cpp
		${Z0_EXAMPLES_DIR}/exemple_1.cpp
)

add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_shaders)
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIR})

include(cmake/libraries.cmake)
