#
# Copyright (c) 2024-2025 Henri Michelon
#
# This software is released under the MIT License.
# https://opensource.org/licenses/MIT
#
###### Detect Vulkan SDK
message(NOTICE "Searching for Vulkan SDK")
find_package(Vulkan REQUIRED)
target_include_directories(${Z0_TARGET} PUBLIC ${Vulkan_INCLUDE_DIRS})

###### Using GLM for maths
add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)
add_compile_definitions(GLM_GTC_constants)
add_compile_definitions(GLM_FORCE_DEPTH_ZERO_TO_ONE)
message(NOTICE "Fetching GLM from https://github.com/g-truc/glm ...")
FetchContent_Declare(
        fetch_glm
        GIT_REPOSITORY https://github.com/g-truc/glm
        GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(fetch_glm)
set(GLM_DIR ${CMAKE_BINARY_DIR}/_deps/fetch_glm-src/glm)
# compile GLM as a module
message(NOTICE "Building glm C++ module...")
add_library(glm-modules STATIC)
target_sources(glm-modules
  PUBLIC
    FILE_SET moduleStd
    TYPE CXX_MODULES
    BASE_DIRS ${GLM_DIR}
    FILES
      ${GLM_DIR}/glm.cppm)
target_link_libraries(glm-modules glm::glm)
target_link_libraries(${Z0_TARGET} glm::glm glm-modules)
if(MSVC)
    target_precompile_headers(${Z0_TARGET} PRIVATE ${GLM_DIR}/glm.hpp ${GLM_DIR}/gtx/quaternion.hpp ${GLM_DIR}/gtx/matrix_decompose.hpp)
endif()

###### Using FastGTLF to load models from binary glTF
message(NOTICE "Fetching FastGTLF from https://github.com/spnda/fastgltf ...")
FetchContent_Declare(
        fetch_fastgltf
        GIT_REPOSITORY https://github.com/spnda/fastgltf
        GIT_TAG        v0.8.0
)
FetchContent_MakeAvailable(fetch_fastgltf)
target_link_libraries(${Z0_TARGET} fastgltf)
#target_link_libraries(${GLTF2ZRES} fastgltf)

###### Using GLFW3 to create a window to transcode image with GPU for gltf2zres
message(NOTICE "Fetching GLFW3 from https://github.com/glfw/glfw ...")
FetchContent_Declare(
        fetch_glfw
        GIT_REPOSITORY https://github.com/glfw/glfw
        GIT_TAG        3.4
)
FetchContent_MakeAvailable(fetch_glfw)
target_link_libraries(${GLTF2ZRES} glfw)

###### Using meshoptimizer to optimize meshes
message(NOTICE "Fetching meshoptimizer from https://github.com/zeux/meshoptimizer ...")
FetchContent_Declare(
        fetch_meshopt
        GIT_REPOSITORY https://github.com/zeux/meshoptimizer
        GIT_TAG        v0.23
)
FetchContent_MakeAvailable(fetch_meshopt)
target_link_libraries(${Z0_TARGET} meshoptimizer)


###### Using KTX to transcode KTX2 to compressed images
#message(NOTICE "Fetching LibKTX from https://github.com/KhronosGroup/KTX-Software ...")
#set(KTX_FEATURE_STATIC_LIBRARY ON CACHE BOOL "Build KTX as a static library" FORCE)
#FetchContent_Declare(
#        fetch_ktx
#        GIT_REPOSITORY https://github.com/KhronosGroup/KTX-Software
#        GIT_TAG        v4.3.2
#)
#FetchContent_MakeAvailable(fetch_ktx)
#target_link_libraries(${Z0_TARGET} ktx)
#target_include_directories(${Z0_TARGET} PRIVATE ${fetch_ktx_SOURCE_DIR}/include)

message(NOTICE "Fetching Jolt Physics from https://github.com/jrouwe/JoltPhysics ...")
include(cmake/jolt.cmake)
