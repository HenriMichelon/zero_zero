#include <z0/input.h>
#include <z0/nodes/camera.h>
#include <z0/nodes/mesh_instance.h>
#include "z0/nodes/skybox.h"
#include "z0/loader.h"

#include "ui.h"

void UIMainScene::onReady() {
    auto camera = make_shared<Camera>();
    addChild(camera);
    auto skybox = make_shared<Skybox>("examples/textures/sky", ".jpg");
    addChild(skybox);
    auto sphere = Loader::loadModelFromFile("examples/models/sphere.glb");
    sphere->setPosition({0.0f, 0.0f, -5.0f});
    addChild(sphere);
}

void UIMainScene::onProcess(float alpha) {
}