#include <z0/input.h>
#include <z0/nodes/camera.h>
#include <z0/nodes/mesh_instance.h>
#include "z0/nodes/skybox.h"
#include "z0/loader.h"
#include "z0/application.h"

#include "ui.h"

void UIMainScene::onReady() {
    auto camera = make_shared<Camera>();
    addChild(camera);
    auto skybox = make_shared<Skybox>("examples/textures/sky", ".jpg");
    addChild(skybox);
    sphere = Loader::loadModelFromFile("examples/models/sphere.glb");
    sphere->setPosition({0.0f, 0.0f, -5.0f});
    addChild(sphere);
    auto window1 = make_shared<GWindow>(GRect{250, 975, 500, 25});
    window1->setBgColor({1.0, 0.647, 0.0, 1.0});
    Application::add(window1);
    auto window2 = make_shared<GWindow>(GRect{250, 750, 500, 500});
    window2->setBgColor({1.0, 0.647, 0.0, 0.1});
    Application::add(window2);
}

void UIMainScene::onPhysicsProcess(float delta) {
    auto angle = delta * radians(90.0f) / 2;
    sphere->rotateY(angle);
}