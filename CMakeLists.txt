cmake_minimum_required(VERSION 3.29)
cmake_policy(SET CMP0002 NEW)

project(ZeroZero)

#add_compile_definitions(NDEBUG)
#add_compile_definitions(USE_PCH)
#add_compile_definitions(DISABLE_LOG)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Engine sources & build directories
set(Z0_INCLUDE_DIR ${INCLUDE_DIR}/z0)
set(Z0_ENGINE_DIR ${SRC_DIR}/z0)
set(Z0_SHADERS_DIR ${Z0_ENGINE_DIR}/shaders)
set(Z0_SHADERS_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
file(MAKE_DIRECTORY ${Z0_SHADERS_BUILD_DIR})

include(FetchContent)
include(cmake/shaders.cmake)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_COMPILE_WARNING_AS_ERROR OFF) # disabled due to fastgltf and VMA
set(CMAKE_CXX_EXTENSIONS OFF)
#add_compile_definitions(VULKAN_STATS) # print some stats at the end of the execution
if(MINGW)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
endif()
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

# compile GLSL shaders
file(GLOB_RECURSE Z0_GLSL_SOURCE_FILES
		"${Z0_SHADERS_DIR}/*.frag"
		"${Z0_SHADERS_DIR}/*.vert"
)
add_shaders(${PROJECT_NAME}_shaders ${Z0_SHADERS_BUILD_DIR} ${Z0_SHADERS_DIR} ${Z0_GLSL_SOURCE_FILES})

# engine library
add_library(${PROJECT_NAME} SHARED
		${Z0_ENGINE_DIR}/tools.cpp
		${Z0_ENGINE_DIR}/object.cpp
		${Z0_ENGINE_DIR}/window.cpp
		${Z0_ENGINE_DIR}/application.cpp
		${Z0_ENGINE_DIR}/device.cpp
		${Z0_ENGINE_DIR}/buffer.cpp
		${Z0_ENGINE_DIR}/stats.cpp
		${Z0_ENGINE_DIR}/descriptors.cpp
		${Z0_ENGINE_DIR}/loader.cpp
		${Z0_ENGINE_DIR}/shader.cpp
		${Z0_ENGINE_DIR}/input_event.cpp
		${Z0_ENGINE_DIR}/input.cpp
		${Z0_ENGINE_DIR}/rect.cpp
		${Z0_ENGINE_DIR}/signal.cpp
		${Z0_ENGINE_DIR}/physics.cpp
		${Z0_ENGINE_DIR}/mappings.h
		${Z0_ENGINE_DIR}/libraries.cpp

		${Z0_ENGINE_DIR}/framebuffers/base_frame_buffer.cpp
		${Z0_ENGINE_DIR}/framebuffers/sampled_frame_buffer.cpp
		${Z0_ENGINE_DIR}/framebuffers/depth_frame_buffer.cpp
		${Z0_ENGINE_DIR}/framebuffers/color_frame_buffer.cpp
		${Z0_ENGINE_DIR}/framebuffers/color_frame_buffer_hdr.cpp
		${Z0_ENGINE_DIR}/framebuffers/shadow_map_frame_buffer.cpp

		${Z0_ENGINE_DIR}/renderers/base_renderpass.cpp
		${Z0_ENGINE_DIR}/renderers/scene_renderer.cpp
		${Z0_ENGINE_DIR}/renderers/base_models_renderer.cpp
		${Z0_ENGINE_DIR}/renderers/skybox_renderer.cpp
		${Z0_ENGINE_DIR}/renderers/vector_renderer.cpp
		${Z0_ENGINE_DIR}/renderers/shadowmap_renderer.cpp
		${Z0_ENGINE_DIR}/renderers/base_postprocessing_renderer.cpp
		${Z0_ENGINE_DIR}/renderers/simple_postprocessing_renderer.cpp

		${Z0_ENGINE_DIR}/nodes/node.cpp
		${Z0_ENGINE_DIR}/nodes/mesh_instance.cpp
		${Z0_ENGINE_DIR}/nodes/camera.cpp
		${Z0_ENGINE_DIR}/nodes/physics_body.cpp
		${Z0_ENGINE_DIR}/nodes/rigid_body.cpp
		${Z0_ENGINE_DIR}/nodes/static_body.cpp
		${Z0_ENGINE_DIR}/nodes/skybox.cpp
		${Z0_ENGINE_DIR}/nodes/kinematic_body.cpp
		${Z0_ENGINE_DIR}/nodes/character.cpp
		${Z0_ENGINE_DIR}/nodes/collision_object.cpp
		${Z0_ENGINE_DIR}/nodes/collision_area.cpp
		${Z0_ENGINE_DIR}/nodes/raycast.cpp
		${Z0_ENGINE_DIR}/nodes/light.cpp
		${Z0_ENGINE_DIR}/nodes/directional_light.cpp
		${Z0_ENGINE_DIR}/nodes/omni_light.cpp
		${Z0_ENGINE_DIR}/nodes/spot_light.cpp

		${Z0_ENGINE_DIR}/resources/resource.cpp
		${Z0_ENGINE_DIR}/resources/image.cpp
		${Z0_ENGINE_DIR}/resources/texture.cpp
		${Z0_ENGINE_DIR}/resources/mesh.cpp
		${Z0_ENGINE_DIR}/resources/material.cpp
		${Z0_ENGINE_DIR}/resources/shape.cpp
		${Z0_ENGINE_DIR}/resources/convex_hull_shape.cpp
		${Z0_ENGINE_DIR}/resources/static_compound_shape.cpp
		${Z0_ENGINE_DIR}/resources/mesh_shape.cpp
		${Z0_ENGINE_DIR}/resources/cubemap.cpp
		${Z0_ENGINE_DIR}/resources/font.cpp

		${Z0_ENGINE_DIR}/gui/gwindow.cpp
		${Z0_ENGINE_DIR}/gui/gwidget.cpp
		${Z0_ENGINE_DIR}/gui/gevent.cpp
		${Z0_ENGINE_DIR}/gui/gmanager.cpp
		${Z0_ENGINE_DIR}/gui/gstyle.cpp
		${Z0_ENGINE_DIR}/gui/gstyle_classic.cpp
		${Z0_ENGINE_DIR}/gui/gstyle_classic_resource.cpp
		${Z0_ENGINE_DIR}/gui/gline.cpp
		${Z0_ENGINE_DIR}/gui/gbox.cpp
		${Z0_ENGINE_DIR}/gui/gbutton.cpp
		${Z0_ENGINE_DIR}/gui/gcheck_widget.cpp
		${Z0_ENGINE_DIR}/gui/gtoggle_button.cpp
		${Z0_ENGINE_DIR}/gui/gtext.cpp
		${Z0_ENGINE_DIR}/gui/gframe.cpp
		${Z0_ENGINE_DIR}/gui/gstyle_classic.h
		${Z0_ENGINE_DIR}/gui/gstyle_classic_resource.h
)

add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_shaders)
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIR})
add_compile_definitions(JPH_DISABLE_CUSTOM_ALLOCATOR)
target_precompile_headers(${PROJECT_NAME} PUBLIC
		${Z0_INCLUDE_DIR}/libraries.h
		${Z0_INCLUDE_DIR}/z0.h
)
if(WIN32)
	target_link_libraries(${PROJECT_NAME} Xinput dinput8 dxguid)
endif()


include(cmake/jolt.cmake)
include(cmake/libraries.cmake)
