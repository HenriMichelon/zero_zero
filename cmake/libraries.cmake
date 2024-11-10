#
# Copyright (c) 2024 Henri Michelon
#
# This software is released under the MIT License.
# https://opensource.org/licenses/MIT
#
###### Detect Vulkan SDK
message(NOTICE "Searching for Vulkan SDK")
find_package(Vulkan REQUIRED)
target_include_directories(${Z0_TARGET} PUBLIC ${Vulkan_INCLUDE_DIRS})
#include(cmake/compressonator.cmake)

###### Using Volk to load Vulkan functions
message(NOTICE "Fetching volk from https://github.com/zeux/volk ...")
FetchContent_Declare(
        fetch_volk
        GIT_REPOSITORY https://github.com/zeux/volk
        GIT_TAG        1.3.295
)
FetchContent_MakeAvailable(fetch_volk)
if (WIN32)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
endif()
target_link_libraries(${Z0_TARGET} volk)

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
#target_link_libraries(${GLB2ZSCENE_TARGET} fastgltf)

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
