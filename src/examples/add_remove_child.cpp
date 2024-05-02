#include "add_remove_child.h"
#include <z0/application.h>
#include <z0/input.h>
#include <z0/loader.h>

void AddRemoveChildMainScene::onReady() {
    camera1 = make_shared<Camera>("Camera 1");
    camera1->setPosition({0.0f, 0.0f, 1.0f});
    addChild(camera1);
    camera2 = make_shared<Camera>("Camera 2");
    camera2->rotateX(radians(-90.0));
    camera2->setPosition({0.0f, 10.0f, -10.0f});
    addChild(camera2);

    crateModel = Loader::loadModelFromFile("models/crate.glb");
    crateModel->setPosition({-5.0f, 0.0f, -10.0f});
    addChild(crateModel);
    crates.push_back(crateModel);

    printTree(cout);
}

void AddRemoveChildMainScene::onProcess(float alpha) {
    if (Input::isKeyJustPressed(KEY_KP_ADD)) {
        auto crate2 = crateModel->duplicate();
        crate2->setPosition({rand() % 10 - 5, rand() % 10 - 5, -10.0f});
        if (addChild(crate2)) crates.push_back(crate2);
    }
    if (Input::isKeyJustPressed(KEY_KP_SUBTRACT)) {
        if (removeChild(crates.back())) crates.pop_back();
    }
    if (Input::isKeyJustPressed(KEY_BACKSPACE)) {
        if (camera1->isActive()) {
            Application::get().activateCamera(camera2);
        } else {
            Application::get().activateCamera(camera1);
        }

    }
}

void AddRemoveChildMainScene::onPhysicsProcess(float delta) {
    auto angle = delta * radians(90.0f) / 2;
    for (auto& crate: crates) {
        crate->rotateY(angle);
        crate->rotateX(angle);
    }
}