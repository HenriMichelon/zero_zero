cmake_minimum_required(VERSION 3.22)
cmake_policy(SET CMP0002 NEW)

project(ZeroZero)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Engine sources & build directories
set(Z0_INCLUDE_DIR ${INCLUDE_DIR}/z0)
set(Z0_ENGINE_DIR ${SRC_DIR}/z0)
set(Z0_SHADERS_DIR ${Z0_ENGINE_DIR}/shaders)
set(Z0_SHADERS_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
make_directory(${Z0_SHADERS_BUILD_DIR})

# Examples sources & build directories
set(Z0_EXAMPLES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/examples/src)
set(Z0_EXAMPLES_SHADERS_DIR ${Z0_EXAMPLES_DIR}/shaders)
set(Z0_EXAMPLES_SHADERS_BUILD_DIR ${Z0_SHADERS_BUILD_DIR}/examples)
make_directory(${Z0_EXAMPLES_SHADERS_BUILD_DIR})

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
add_shaders(${PROJECT_NAME}_z0_shaders  ${Z0_SHADERS_BUILD_DIR} ${Z0_GLSL_SOURCE_FILES})
file(GLOB_RECURSE Z0_EXAMPLES_GLSL_SOURCE_FILES
		"${Z0_EXAMPLES_SHADERS_DIR}/*.frag"
		"${Z0_EXAMPLES_SHADERS_DIR}/*.vert"
)
add_shaders(${PROJECT_NAME}_z0_examples_shaders  ${Z0_EXAMPLES_SHADERS_BUILD_DIR} ${Z0_EXAMPLES_GLSL_SOURCE_FILES})

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
		${Z0_INCLUDE_DIR}/descriptors.h
		${Z0_INCLUDE_DIR}/color.h
		${Z0_INCLUDE_DIR}/loader.h
		${Z0_INCLUDE_DIR}/shader.h
		${Z0_INCLUDE_DIR}/input_event.h
		${Z0_INCLUDE_DIR}/input.h
		${Z0_INCLUDE_DIR}/physics.h
		${Z0_INCLUDE_DIR}/rect.h
		${Z0_INCLUDE_DIR}/libraries.h
		${Z0_INCLUDE_DIR}/base.h
		${Z0_INCLUDE_DIR}/z0.h

		${Z0_INCLUDE_DIR}/framebuffers/base_frame_buffer.h
		${Z0_INCLUDE_DIR}/framebuffers/depth_frame_buffer.h
		${Z0_INCLUDE_DIR}/framebuffers/color_frame_buffer.h
		${Z0_INCLUDE_DIR}/framebuffers/color_frame_buffer_hdr.h

		${Z0_INCLUDE_DIR}/renderers/base_renderer.h
		${Z0_INCLUDE_DIR}/renderers/base_models_renderer.h
		${Z0_INCLUDE_DIR}/renderers/base_renderpass.h
		${Z0_INCLUDE_DIR}/renderers/scene_renderer.h
		${Z0_INCLUDE_DIR}/renderers/skybox_renderer.h
		${Z0_INCLUDE_DIR}/renderers/vector_renderer.h

		${Z0_INCLUDE_DIR}/nodes/node.h
		${Z0_INCLUDE_DIR}/nodes/mesh_instance.h
		${Z0_INCLUDE_DIR}/nodes/camera.h
		${Z0_INCLUDE_DIR}/nodes/physics_body.h
		${Z0_INCLUDE_DIR}/nodes/rigid_body.h
		${Z0_INCLUDE_DIR}/nodes/static_body.h
		${Z0_INCLUDE_DIR}/nodes/skybox.h
		${Z0_INCLUDE_DIR}/nodes/physics_node.h
		${Z0_INCLUDE_DIR}/nodes/character.h
		${Z0_INCLUDE_DIR}/nodes/kinematic_body.h
		${Z0_INCLUDE_DIR}/nodes/raycast.h

		${Z0_INCLUDE_DIR}/resources/resource.h
		${Z0_INCLUDE_DIR}/resources/image.h
		${Z0_INCLUDE_DIR}/resources/texture.h
		${Z0_INCLUDE_DIR}/resources/material.h
		${Z0_INCLUDE_DIR}/resources/mesh.h
		${Z0_INCLUDE_DIR}/resources/shape.h
		${Z0_INCLUDE_DIR}/resources/cubemap.h
		${Z0_INCLUDE_DIR}/resources/font.h

		${Z0_INCLUDE_DIR}/gui/gwindow.h
		${Z0_INCLUDE_DIR}/gui/gwidget.h
		${Z0_INCLUDE_DIR}/gui/gevent.h
		${Z0_INCLUDE_DIR}/gui/gresource.h
		${Z0_INCLUDE_DIR}/gui/gstyle.h
		${Z0_INCLUDE_DIR}/gui/gmanager.h
		${Z0_INCLUDE_DIR}/gui/gpanel.h
		${Z0_INCLUDE_DIR}/gui/gline.h
		${Z0_INCLUDE_DIR}/gui/gbutton.h
		${Z0_INCLUDE_DIR}/gui/gbox.h
		${Z0_INCLUDE_DIR}/gui/gtoggle_button.h
		${Z0_INCLUDE_DIR}/gui/gcheck_widget.h
		${Z0_INCLUDE_DIR}/gui/gtext.h
		${Z0_INCLUDE_DIR}/gui/gframe.h

		${Z0_ENGINE_DIR}/tools.cpp
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
		${Z0_ENGINE_DIR}/physics.cpp
		${Z0_ENGINE_DIR}/mappings.h
		${Z0_ENGINE_DIR}/libraries.cpp

		${Z0_ENGINE_DIR}/framebuffers/base_frame_buffer.cpp
		${Z0_ENGINE_DIR}/framebuffers/depth_frame_buffer.cpp
		${Z0_ENGINE_DIR}/framebuffers/color_frame_buffer.cpp
		${Z0_ENGINE_DIR}/framebuffers/color_frame_buffer_hdr.cpp

		${Z0_ENGINE_DIR}/renderers/base_renderpass.cpp
		${Z0_ENGINE_DIR}/renderers/scene_renderer.cpp
		${Z0_ENGINE_DIR}/renderers/base_models_renderer.cpp
		${Z0_ENGINE_DIR}/renderers/skybox_renderer.cpp
		${Z0_ENGINE_DIR}/renderers/vector_renderer.cpp

		${Z0_ENGINE_DIR}/nodes/node.cpp
		${Z0_ENGINE_DIR}/nodes/mesh_instance.cpp
		${Z0_ENGINE_DIR}/nodes/camera.cpp
		${Z0_ENGINE_DIR}/nodes/physics_body.cpp
		${Z0_ENGINE_DIR}/nodes/rigid_body.cpp
		${Z0_ENGINE_DIR}/nodes/static_body.cpp
		${Z0_ENGINE_DIR}/nodes/skybox.cpp
		${Z0_ENGINE_DIR}/nodes/kinematic_body.cpp
		${Z0_ENGINE_DIR}/nodes/character.cpp
		${Z0_ENGINE_DIR}/nodes/physics_node.cpp
		${Z0_ENGINE_DIR}/nodes/raycast.cpp

		${Z0_ENGINE_DIR}/resources/resource.cpp
		${Z0_ENGINE_DIR}/resources/image.cpp
		${Z0_ENGINE_DIR}/resources/texture.cpp
		${Z0_ENGINE_DIR}/resources/mesh.cpp
		${Z0_ENGINE_DIR}/resources/material.cpp
		${Z0_ENGINE_DIR}/resources/shape.cpp
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

		${Z0_EXAMPLES_DIR}/example.cpp
		${Z0_EXAMPLES_DIR}/triangle.h
		${Z0_EXAMPLES_DIR}/triangle.cpp
		${Z0_EXAMPLES_DIR}/add_remove_child.h
		${Z0_EXAMPLES_DIR}/add_remove_child.cpp
		${Z0_EXAMPLES_DIR}/physics.h
		${Z0_EXAMPLES_DIR}/physics.cpp
		${Z0_EXAMPLES_DIR}/ui.h
		${Z0_EXAMPLES_DIR}/ui.cpp
		${Z0_EXAMPLES_DIR}/topbar.h
		${Z0_EXAMPLES_DIR}/topbar.cpp
		${Z0_EXAMPLES_DIR}/player.h
		${Z0_EXAMPLES_DIR}/player.cpp
		${Z0_EXAMPLES_DIR}/layers.h
		${Z0_EXAMPLES_DIR}/menu.h
		${Z0_EXAMPLES_DIR}/menu.cpp
)

add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_z0_shaders  ${PROJECT_NAME}_z0_examples_shaders)
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIR})
target_precompile_headers(${PROJECT_NAME} PRIVATE
		${Z0_INCLUDE_DIR}/libraries.h
		${Z0_INCLUDE_DIR}/base.h
		${Z0_INCLUDE_DIR}/z0.h
)

if(WIN32)
	target_link_libraries(${PROJECT_NAME} Xinput dinput8 dxguid)
endif()

include(cmake/jolt.cmake)
include(cmake/libraries.cmake)
