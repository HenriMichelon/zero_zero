cmake_minimum_required(VERSION 3.29)
cmake_policy(SET CMP0002 NEW)

project(ZeroZero)

#add_compile_definitions(DISABLE_LOG)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(Z0_TARGET ${PROJECT_NAME})

# Engine sources & build directories
set(Z0_INCLUDE_DIR ${INCLUDE_DIR}/z0)
set(Z0_ENGINE_DIR ${SRC_DIR}/z0)
set(Z0_SHADERS_DIR ${Z0_ENGINE_DIR}/shaders)
set(Z0_SHADERS_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
file(MAKE_DIRECTORY ${Z0_SHADERS_BUILD_DIR})

# Tools
set(Z0_TOOL_GLTF2JON "gltf2json")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(MINGW)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
endif()
include(cmake/shaders.cmake)

# compile GLSL shaders
file(GLOB_RECURSE Z0_GLSL_SOURCE_FILES
		"${Z0_SHADERS_DIR}/*.frag"
		"${Z0_SHADERS_DIR}/*.vert"
		"${Z0_SHADERS_DIR}/*.comp"
)
add_shaders(${Z0_TARGET}_shaders ${Z0_SHADERS_BUILD_DIR} ${Z0_SHADERS_DIR} ${Z0_GLSL_SOURCE_FILES})

# engine modules
set(PROJECT_FILES
		${Z0_ENGINE_DIR}/aabb.cppm
		${Z0_ENGINE_DIR}/application.cppm
		${Z0_ENGINE_DIR}/application_config.cppm
		${Z0_ENGINE_DIR}/buffer.cppm
		${Z0_ENGINE_DIR}/color.cppm
		${Z0_ENGINE_DIR}/constants.cppm
		${Z0_ENGINE_DIR}/descriptors.cppm
		${Z0_ENGINE_DIR}/device.cppm
		${Z0_ENGINE_DIR}/frustum_culling.cppm
		${Z0_ENGINE_DIR}/input.cppm
		${Z0_ENGINE_DIR}/input_event.cppm
		${Z0_ENGINE_DIR}/loader.cppm
		${Z0_ENGINE_DIR}/object.cppm
		${Z0_ENGINE_DIR}/physics.cppm
		${Z0_ENGINE_DIR}/shader.cppm
		${Z0_ENGINE_DIR}/signal.cppm
		${Z0_ENGINE_DIR}/tools.cppm
		${Z0_ENGINE_DIR}/tween.cppm
		${Z0_ENGINE_DIR}/type_registry.cppm
		${Z0_ENGINE_DIR}/window.cppm
		${Z0_ENGINE_DIR}/z0.cppm

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

		${Z0_ENGINE_DIR}/framebuffers/color.cppm
		${Z0_ENGINE_DIR}/framebuffers/color_hdr.cppm
		${Z0_ENGINE_DIR}/framebuffers/depth.cppm
		${Z0_ENGINE_DIR}/framebuffers/frame_buffer.cppm
		${Z0_ENGINE_DIR}/framebuffers/sampled_frame_buffer.cppm
		${Z0_ENGINE_DIR}/framebuffers/shadowmap.cppm

		${Z0_ENGINE_DIR}/pipelines/compute.cppm
		${Z0_ENGINE_DIR}/pipelines/equirect2cubemap.cppm
		${Z0_ENGINE_DIR}/pipelines/pipeline.cppm

		${Z0_ENGINE_DIR}/renderers/models.cppm
		${Z0_ENGINE_DIR}/renderers/postprocessing.cppm
		${Z0_ENGINE_DIR}/renderers/renderer.cppm
		${Z0_ENGINE_DIR}/renderers/renderpass.cppm
		${Z0_ENGINE_DIR}/renderers/scene.cppm
		${Z0_ENGINE_DIR}/renderers/shadowmap.cppm
		${Z0_ENGINE_DIR}/renderers/simple_postprocessing.cppm
		${Z0_ENGINE_DIR}/renderers/tonemapping.cppm
		${Z0_ENGINE_DIR}/renderers/skybox.cppm
		${Z0_ENGINE_DIR}/renderers/vector.cppm

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

# engine library
add_library(${Z0_TARGET} STATIC
		${Z0_ENGINE_DIR}/aabb.cpp
		${Z0_ENGINE_DIR}/application.cpp
		${Z0_ENGINE_DIR}/buffer.cpp
		${Z0_ENGINE_DIR}/descriptors.cpp
		${Z0_ENGINE_DIR}/device.cpp
		${Z0_ENGINE_DIR}/frustum_culling.cpp
		${Z0_ENGINE_DIR}/input.cpp
		${Z0_ENGINE_DIR}/libraries.cpp
		${Z0_ENGINE_DIR}/loader.cpp
		${Z0_ENGINE_DIR}/object.cpp
		${Z0_ENGINE_DIR}/physics.cpp
		${Z0_ENGINE_DIR}/shader.cpp
		${Z0_ENGINE_DIR}/signal.cpp
		${Z0_ENGINE_DIR}/tools.cpp
		${Z0_ENGINE_DIR}/window.cpp

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

		${Z0_ENGINE_DIR}/framebuffers/color.cpp
		${Z0_ENGINE_DIR}/framebuffers/color_hdr.cpp
		${Z0_ENGINE_DIR}/framebuffers/depth.cpp
		${Z0_ENGINE_DIR}/framebuffers/frame_buffer.cpp
		${Z0_ENGINE_DIR}/framebuffers/shadowmap.cpp

		${Z0_ENGINE_DIR}/pipelines/compute.cpp
		${Z0_ENGINE_DIR}/pipelines/equirect2cubemap.cpp
		${Z0_ENGINE_DIR}/pipelines/pipeline.cpp

		${Z0_ENGINE_DIR}/renderers/models.cpp
		${Z0_ENGINE_DIR}/renderers/postprocessing.cpp
		${Z0_ENGINE_DIR}/renderers/renderpass.cpp
		${Z0_ENGINE_DIR}/renderers/scene.cpp
		${Z0_ENGINE_DIR}/renderers/shadowmap.cpp
		${Z0_ENGINE_DIR}/renderers/simple_postprocessing.cpp
		${Z0_ENGINE_DIR}/renderers/tonemapping.cpp
		${Z0_ENGINE_DIR}/renderers/skybox.cpp
		${Z0_ENGINE_DIR}/renderers/vector.cpp

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

# command line tools
add_executable(${Z0_TOOL_GLTF2JON}
		${Z0_ENGINE_DIR}/tools/gltf2json/gltf2json.cpp
)
target_link_libraries(${Z0_TOOL_GLTF2JON} ${PROJECT_NAME})

#https://github.com/llvm/llvm-project/issues/105994
#target_precompile_headers(${PROJECT_NAME} PUBLIC ${Z0_INCLUDE_DIR}/libraries.h)

add_dependencies(${Z0_TARGET} ${Z0_TARGET}_shaders)
target_include_directories(${Z0_TARGET} PUBLIC ${INCLUDE_DIR})
add_compile_definitions(JPH_DISABLE_CUSTOM_ALLOCATOR)
if(WIN32)
	target_link_libraries(${Z0_TARGET} Xinput dinput8 dxguid dxgi)
endif()

include(FetchContent)
include(cmake/jolt.cmake)
include(cmake/libraries.cmake)
include(cmake/std.cmake)
target_link_libraries(${PROJECT_NAME} std-cxx-modules)
target_link_libraries(${Z0_TOOL_GLTF2JON} std-cxx-modules)
