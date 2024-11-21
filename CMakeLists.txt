#
# Copyright (c) 2024 Henri Michelon
#
# This software is released under the MIT License.
# https://opensource.org/licenses/MIT
#
cmake_minimum_required(VERSION 3.29)
cmake_policy(SET CMP0002 NEW)

project(ZeroZero)

#add_compile_definitions(DISABLE_LOG)
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_definitions(GLM_ENABLE_EXPERIMENTAL ON)
add_compile_definitions(GLM_ENABLE_CXX_20 ON)
add_compile_definitions(JPH_DISABLE_CUSTOM_ALLOCATOR)
include(cmake/shaders.cmake)
include(cmake/compile_options.cmake)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(Z0_TARGET ${PROJECT_NAME})

###########################################################################
# Engine shared library
###########################################################################
# Engine sources & build directories
set(Z0_INCLUDE_DIR ${INCLUDE_DIR}/z0)
set(Z0_ENGINE_DIR ${SRC_DIR}/z0)

# engine modules
set(PROJECT_FILES
		${Z0_ENGINE_DIR}/aabb.cppm
		${Z0_ENGINE_DIR}/application.cppm
		${Z0_ENGINE_DIR}/application_config.cppm
		${Z0_ENGINE_DIR}/constants.cppm
		${Z0_ENGINE_DIR}/debug_config.cppm
		${Z0_ENGINE_DIR}/frustum_culling.cppm
		${Z0_ENGINE_DIR}/gltf.cppm
		${Z0_ENGINE_DIR}/input.cppm
		${Z0_ENGINE_DIR}/input_event.cppm
		${Z0_ENGINE_DIR}/loader.cppm
		${Z0_ENGINE_DIR}/object.cppm
		${Z0_ENGINE_DIR}/physics.cppm
		${Z0_ENGINE_DIR}/signal.cppm
		${Z0_ENGINE_DIR}/tools.cppm
		${Z0_ENGINE_DIR}/tween.cppm
		${Z0_ENGINE_DIR}/type_registry.cppm
		${Z0_ENGINE_DIR}/virtual_fs.cppm
		${Z0_ENGINE_DIR}/window.cppm
		${Z0_ENGINE_DIR}/z0.cppm
		${Z0_ENGINE_DIR}/zres.cppm

		${Z0_ENGINE_DIR}/vulkan/application.cppm
		${Z0_ENGINE_DIR}/vulkan/buffer.cppm
		${Z0_ENGINE_DIR}/vulkan/descriptors.cppm
		${Z0_ENGINE_DIR}/vulkan/device.cppm
		${Z0_ENGINE_DIR}/vulkan/instance.cppm
		${Z0_ENGINE_DIR}/vulkan/shader.cppm

		${Z0_ENGINE_DIR}/nodes/animation_player.cppm
		${Z0_ENGINE_DIR}/nodes/camera.cppm
		${Z0_ENGINE_DIR}/nodes/character.cppm
		${Z0_ENGINE_DIR}/nodes/collision_area.cppm
		${Z0_ENGINE_DIR}/nodes/collision_object.cppm
		${Z0_ENGINE_DIR}/nodes/directional_light.cppm
		${Z0_ENGINE_DIR}/nodes/environment.cppm
		${Z0_ENGINE_DIR}/nodes/kinematic_body.cppm
		${Z0_ENGINE_DIR}/nodes/light.cppm
		${Z0_ENGINE_DIR}/nodes/mesh_instance.cppm
		${Z0_ENGINE_DIR}/nodes/node.cppm
		${Z0_ENGINE_DIR}/nodes/omni_light.cppm
		${Z0_ENGINE_DIR}/nodes/physics_body.cppm
		${Z0_ENGINE_DIR}/nodes/raycast.cppm
		${Z0_ENGINE_DIR}/nodes/rigid_body.cppm
		${Z0_ENGINE_DIR}/nodes/skybox.cppm
		${Z0_ENGINE_DIR}/nodes/spot_light.cppm
		${Z0_ENGINE_DIR}/nodes/static_body.cppm
		${Z0_ENGINE_DIR}/nodes/viewport.cppm

		${Z0_ENGINE_DIR}/resources/animation.cppm
		${Z0_ENGINE_DIR}/resources/animation_library.cppm
		${Z0_ENGINE_DIR}/resources/convex_hull_shape.cppm
		${Z0_ENGINE_DIR}/resources/cubemap.cppm
		${Z0_ENGINE_DIR}/resources/font.cppm
		${Z0_ENGINE_DIR}/resources/image.cppm
		${Z0_ENGINE_DIR}/resources/material.cppm
		${Z0_ENGINE_DIR}/resources/mesh.cppm
		${Z0_ENGINE_DIR}/resources/mesh_shape.cppm
		${Z0_ENGINE_DIR}/resources/resource.cppm
		${Z0_ENGINE_DIR}/resources/shape.cppm
		${Z0_ENGINE_DIR}/resources/static_compound_shape.cppm
		${Z0_ENGINE_DIR}/resources/sub_shape.cppm
		${Z0_ENGINE_DIR}/resources/texture.cppm

		${Z0_ENGINE_DIR}/vulkan/resources/cubemap.cppm
		${Z0_ENGINE_DIR}/vulkan/resources/image.cppm
		${Z0_ENGINE_DIR}/vulkan/resources/mesh.cppm

		${Z0_ENGINE_DIR}/vulkan/framebuffers/color.cppm
		${Z0_ENGINE_DIR}/vulkan/framebuffers/color_hdr.cppm
		${Z0_ENGINE_DIR}/vulkan/framebuffers/depth.cppm
		${Z0_ENGINE_DIR}/vulkan/framebuffers/frame_buffer.cppm
		${Z0_ENGINE_DIR}/vulkan/framebuffers/sampled_frame_buffer.cppm
		${Z0_ENGINE_DIR}/vulkan/framebuffers/shadowmap.cppm

		${Z0_ENGINE_DIR}/vulkan/pipelines/compute.cppm
		${Z0_ENGINE_DIR}/vulkan/pipelines/IBL.cppm
		${Z0_ENGINE_DIR}/vulkan/pipelines/pipeline.cppm

		${Z0_ENGINE_DIR}/vulkan/renderers/debug.cppm
		${Z0_ENGINE_DIR}/vulkan/renderers/models.cppm
		${Z0_ENGINE_DIR}/vulkan/renderers/postprocessing.cppm
		${Z0_ENGINE_DIR}/vulkan/renderers/renderer.cppm
		${Z0_ENGINE_DIR}/vulkan/renderers/renderpass.cppm
		${Z0_ENGINE_DIR}/vulkan/renderers/scene.cppm
		${Z0_ENGINE_DIR}/vulkan/renderers/shadowmap.cppm
		${Z0_ENGINE_DIR}/vulkan/renderers/simple_postprocessing.cppm
		${Z0_ENGINE_DIR}/vulkan/renderers/tonemapping.cppm
		${Z0_ENGINE_DIR}/vulkan/renderers/skybox.cppm
		${Z0_ENGINE_DIR}/vulkan/renderers/vector.cppm

		${Z0_ENGINE_DIR}/gui/color.cppm
		${Z0_ENGINE_DIR}/gui/gresource.cppm
		${Z0_ENGINE_DIR}/gui/gstyle.cppm
		${Z0_ENGINE_DIR}/gui/gstyle_classic.cppm
		${Z0_ENGINE_DIR}/gui/gstyle_classic_resource.cppm
		${Z0_ENGINE_DIR}/gui/gwidget.cppm
		${Z0_ENGINE_DIR}/gui/gpanel.cppm
		${Z0_ENGINE_DIR}/gui/gline.cppm
		${Z0_ENGINE_DIR}/gui/gbox.cppm
		${Z0_ENGINE_DIR}/gui/gbutton.cppm
		${Z0_ENGINE_DIR}/gui/gcheck_widget.cppm
		${Z0_ENGINE_DIR}/gui/gevent.cppm
		${Z0_ENGINE_DIR}/gui/gframe.cppm
		${Z0_ENGINE_DIR}/gui/gvalue_select.cppm
		${Z0_ENGINE_DIR}/gui/gtext.cppm
		${Z0_ENGINE_DIR}/gui/gtext_edit.cppm
		${Z0_ENGINE_DIR}/gui/gtoggle_button.cppm
		${Z0_ENGINE_DIR}/gui/gscroll_bar.cppm
		${Z0_ENGINE_DIR}/gui/gtree_view.cppm
		${Z0_ENGINE_DIR}/gui/gwindow.cppm
		${Z0_ENGINE_DIR}/gui/gmanager.cppm
		${Z0_ENGINE_DIR}/gui/rect.cppm
)

# engine sources
add_library(${Z0_TARGET} STATIC
		${Z0_ENGINE_DIR}/aabb.cpp
		${Z0_ENGINE_DIR}/application.cpp
		${Z0_ENGINE_DIR}/frustum_culling.cpp
		${Z0_ENGINE_DIR}/gltf.cpp
		${Z0_ENGINE_DIR}/input.cpp
		${Z0_ENGINE_DIR}/libraries.cpp
		${Z0_ENGINE_DIR}/loader.cpp
		${Z0_ENGINE_DIR}/object.cpp
		${Z0_ENGINE_DIR}/physics.cpp
		${Z0_ENGINE_DIR}/signal.cpp
		${Z0_ENGINE_DIR}/tools.cpp
		${Z0_ENGINE_DIR}/virtual_fs.cpp
		${Z0_ENGINE_DIR}/window.cpp
		${Z0_ENGINE_DIR}/zres.cpp

		${Z0_ENGINE_DIR}/vulkan/application.cpp
		${Z0_ENGINE_DIR}/vulkan/buffer.cpp
		${Z0_ENGINE_DIR}/vulkan/descriptors.cpp
		${Z0_ENGINE_DIR}/vulkan/device.cpp
		${Z0_ENGINE_DIR}/vulkan/instance.cpp
		${Z0_ENGINE_DIR}/vulkan/shader.cpp
		${Z0_ENGINE_DIR}/vulkan/zres.cpp

		${Z0_ENGINE_DIR}/nodes/animation_player.cpp
		${Z0_ENGINE_DIR}/nodes/camera.cpp
		${Z0_ENGINE_DIR}/nodes/character.cpp
		${Z0_ENGINE_DIR}/nodes/collision_area.cpp
		${Z0_ENGINE_DIR}/nodes/collision_object.cpp
		${Z0_ENGINE_DIR}/nodes/directional_light.cpp
		${Z0_ENGINE_DIR}/nodes/environment.cpp
		${Z0_ENGINE_DIR}/nodes/kinematic_body.cpp
		${Z0_ENGINE_DIR}/nodes/light.cpp
		${Z0_ENGINE_DIR}/nodes/mesh_instance.cpp
		${Z0_ENGINE_DIR}/nodes/node.cpp
		${Z0_ENGINE_DIR}/nodes/omni_light.cpp
		${Z0_ENGINE_DIR}/nodes/physics_body.cpp
		${Z0_ENGINE_DIR}/nodes/raycast.cpp
		${Z0_ENGINE_DIR}/nodes/rigid_body.cpp
		${Z0_ENGINE_DIR}/nodes/skybox.cpp
		${Z0_ENGINE_DIR}/nodes/spot_light.cpp
		${Z0_ENGINE_DIR}/nodes/static_body.cpp
		${Z0_ENGINE_DIR}/nodes/viewport.cpp

		${Z0_ENGINE_DIR}/resources/animation.cpp
		${Z0_ENGINE_DIR}/resources/convex_hull_shape.cpp
		${Z0_ENGINE_DIR}/resources/cubemap.cpp
		${Z0_ENGINE_DIR}/resources/font.cpp
		${Z0_ENGINE_DIR}/resources/image.cpp
		${Z0_ENGINE_DIR}/resources/material.cpp
		${Z0_ENGINE_DIR}/resources/mesh.cpp
		${Z0_ENGINE_DIR}/resources/mesh_shape.cpp
		${Z0_ENGINE_DIR}/resources/resource.cpp
		${Z0_ENGINE_DIR}/resources/shape.cpp
		${Z0_ENGINE_DIR}/resources/static_compound_shape.cpp
		${Z0_ENGINE_DIR}/resources/texture.cpp

		${Z0_ENGINE_DIR}/vulkan/resources/cubemap.cpp
		${Z0_ENGINE_DIR}/vulkan/resources/image.cpp
		${Z0_ENGINE_DIR}/vulkan/resources/font.cpp
		${Z0_ENGINE_DIR}/vulkan/resources/mesh.cpp

		${Z0_ENGINE_DIR}/vulkan/framebuffers/color.cpp
		${Z0_ENGINE_DIR}/vulkan/framebuffers/color_hdr.cpp
		${Z0_ENGINE_DIR}/vulkan/framebuffers/depth.cpp
		${Z0_ENGINE_DIR}/vulkan/framebuffers/frame_buffer.cpp
		${Z0_ENGINE_DIR}/vulkan/framebuffers/shadowmap.cpp

		${Z0_ENGINE_DIR}/vulkan/pipelines/compute.cpp
		${Z0_ENGINE_DIR}/vulkan/pipelines/IBL.cpp
		${Z0_ENGINE_DIR}/vulkan/pipelines/pipeline.cpp

		${Z0_ENGINE_DIR}/vulkan/renderers/debug.cpp
		${Z0_ENGINE_DIR}/vulkan/renderers/models.cpp
		${Z0_ENGINE_DIR}/vulkan/renderers/postprocessing.cpp
		${Z0_ENGINE_DIR}/vulkan/renderers/renderpass.cpp
		${Z0_ENGINE_DIR}/vulkan/renderers/scene.cpp
		${Z0_ENGINE_DIR}/vulkan/renderers/shadowmap.cpp
		${Z0_ENGINE_DIR}/vulkan/renderers/simple_postprocessing.cpp
		${Z0_ENGINE_DIR}/vulkan/renderers/tonemapping.cpp
		${Z0_ENGINE_DIR}/vulkan/renderers/skybox.cpp
		${Z0_ENGINE_DIR}/vulkan/renderers/vector.cpp

		${Z0_ENGINE_DIR}/gui/gmanager.cpp
		${Z0_ENGINE_DIR}/gui/gstyle.cpp
		${Z0_ENGINE_DIR}/gui/gwidget.cpp
		${Z0_ENGINE_DIR}/gui/gwindow.cpp
)
target_sources(${Z0_TARGET}
	PUBLIC
	FILE_SET CXX_MODULES
	FILES
		${PROJECT_FILES}
)
compile_options(${Z0_TARGET})

# GLSL shaders & dependencies
set(Z0_SHADERS_DIR ${Z0_ENGINE_DIR}/shaders)
set(Z0_SHADERS_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
file(MAKE_DIRECTORY ${Z0_SHADERS_BUILD_DIR})
file(GLOB_RECURSE Z0_GLSL_SOURCE_FILES
		"${Z0_SHADERS_DIR}/*.frag"
		"${Z0_SHADERS_DIR}/*.vert"
		"${Z0_SHADERS_DIR}/*.comp"
)
add_shaders(${Z0_TARGET}_shaders ${Z0_SHADERS_BUILD_DIR} ${Z0_SHADERS_DIR} ${Z0_GLSL_SOURCE_FILES})
add_dependencies(${Z0_TARGET} ${Z0_TARGET}_shaders)
target_include_directories(${Z0_TARGET} PUBLIC ${INCLUDE_DIR})
if(WIN32)
	target_link_libraries(${Z0_TARGET} Xinput dinput8 dxguid dxgi)
endif()

#https://github.com/llvm/llvm-project/issues/105994
#target_precompile_headers(${PROJECT_NAME} PUBLIC ${Z0_INCLUDE_DIR}/libraries.h)

###########################################################################
# gltf2zres tool
###########################################################################
set(GLTF2ZRES gltf2zres)
set(GLTF2ZRES_SRC_DIR ${SRC_DIR}/tools/gltf2zres)
set(GLTF2ZRES_INCLUDE_DIR ${GLTF2ZRES_SRC_DIR})
add_executable(${GLTF2ZRES}
		${GLTF2ZRES_SRC_DIR}/main.cpp
		${GLTF2ZRES_SRC_DIR}/image.cpp
		${GLTF2ZRES_SRC_DIR}/converter.cpp
		${GLTF2ZRES_SRC_DIR}/glad/glad.c
)
target_sources(${GLTF2ZRES}
		PUBLIC
		FILE_SET CXX_MODULES
		FILES
		${GLTF2ZRES_SRC_DIR}/miplevel.cppm
		${GLTF2ZRES_SRC_DIR}/image.cppm
		${GLTF2ZRES_SRC_DIR}/converter.cppm
)
compile_options(${GLTF2ZRES})
target_include_directories(${GLTF2ZRES} PUBLIC ${GLTF2ZRES_INCLUDE_DIR} ${Z0_INCLUDE_DIR})
target_link_libraries(${GLTF2ZRES} ${Z0_TARGET})

###########################################################################
# global target
###########################################################################
add_custom_target(ZeroZeroEngine ALL)
add_dependencies(ZeroZeroEngine ${Z0_TARGET} ${GLTF2ZRES})

include(FetchContent)
include(cmake/std.cmake)
include(cmake/libraries.cmake)
