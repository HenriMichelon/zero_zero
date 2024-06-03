#pragma once

#include <z0/z0.h>
#ifndef USE_PCH
#include <z0/resources/image.h>
#include <z0/resources/texture.h>
#include <z0/resources/material.h>
#include <z0/resources/mesh.h>
#include <z0/resources/cubemap.h>
#include <z0/resources/font.h>
#include <z0/resources/shape.h>

#include <z0/nodes/node.h>
#include <z0/application.h>
#include <z0/input.h>
#include <z0/loader.h>

#include <z0/nodes/camera.h>
#include <z0/nodes/mesh_instance.h>
#include <z0/nodes/physics_node.h>
#include <z0/nodes/physics_body.h>
#include <z0/nodes/rigid_body.h>
#include <z0/nodes/kinematic_body.h>
#include <z0/nodes/static_body.h>
#include <z0/nodes/character.h>
#include <z0/nodes/skybox.h>
#include <z0/nodes/raycast.h>

#include <z0/renderers/base_renderpass.h>
#include <z0/renderers/skybox_renderer.h>
#include <z0/renderers/base_models_renderer.h>
#include <z0/renderers/scene_renderer.h>
#include <z0/renderers/vector_renderer.h>

#include <z0/gui/gresource.h>
#include <z0/gui/gstyle.h>
#include <z0/gui/gevent.h>
#include <z0/gui/gwindow.h>
#include <z0/gui/gmanager.h>
#include <z0/gui/gwidget.h>
#include <z0/gui/gbox.h>
#include <z0/gui/gpanel.h>
#include <z0/gui/gtext.h>
#include <z0/gui/gbutton.h>
#include <z0/gui/gcheck_widget.h>
#include <z0/gui/gtoggle_button.h>
#include <z0/gui/gline.h>
#include <z0/gui/gframe.h>
#endif
using namespace z0;