###### Auto detect Vulkan SDK
find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
# compile Vulkan as a module
add_library(vulkan-modules STATIC)
target_include_directories(vulkan-modules PUBLIC ${Vulkan_INCLUDE_DIRS})
target_sources(vulkan-modules
  PUBLIC
    FILE_SET moduleStd
    TYPE CXX_MODULES
    BASE_DIRS ${Vulkan_INCLUDE_DIRS}
    FILES
      ${Vulkan_INCLUDE_DIRS}/vulkan/vulkan.cppm)
target_link_libraries(${PROJECT_NAME} vulkan-modules)

###### Using Volk to load Vulkan functions
FetchContent_Declare(
        fetch_volk
        GIT_REPOSITORY https://github.com/zeux/volk
        GIT_TAG        vulkan-sdk-1.3.283.0
)
FetchContent_MakeAvailable(fetch_volk)
if (WIN32)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
endif()
target_link_libraries(${PROJECT_NAME} volk)

###### Using GLM for maths
FetchContent_Declare(
        fetch_glm
        GIT_REPOSITORY https://github.com/g-truc/glm
        GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(fetch_glm)
set(GLM_DIR ${CMAKE_BINARY_DIR}/_deps/fetch_glm-src/glm)
# compile GLM as a module
add_library(glm-modules STATIC)
target_sources(glm-modules
  PUBLIC
    FILE_SET moduleStd
    TYPE CXX_MODULES
    BASE_DIRS ${GLM_DIR}
    FILES
      ${GLM_DIR}/glm.cppm)
target_link_libraries(glm-modules glm::glm)
target_link_libraries(${PROJECT_NAME} glm::glm glm-modules)

###### Using FastGTLF to load models
FetchContent_Declare(
        fetch_fastgltf
        GIT_REPOSITORY https://github.com/spnda/fastgltf
        GIT_TAG        v0.7.2
)
FetchContent_MakeAvailable(fetch_fastgltf)
target_link_libraries(${PROJECT_NAME} fastgltf)
