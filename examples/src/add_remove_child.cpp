#include "includes.h"
#include "add_remove_child.h"

void AddRemoveChildMainScene::onReady() {
    camera1 = make_shared<Camera>("Camera 1");
    camera1->setPosition({0.0f, 0.0f, 1.0f});
    addChild(camera1);
    camera2 = make_shared<Camera>("Camera 2");
    camera2->rotateX(radians(-90.0));
    camera2->setPosition({0.0f, 10.0f, -10.0f});
    addChild(camera2);

    sphereModel = Loader::loadModelFromFile("examples/models/sphere.glb");
    crateModel = Loader::loadModelFromFile("examples/models/crate.glb");

    /*crateModel->setPosition({-5.0f, 0.0f, -10.0f});
    addChild(crateModel);
    rotatingNodes.push_back(crateModel);

    sphereModel->setPosition({5.0f, 0.0f, -10.0f});
    addChild(sphereModel);
    rotatingNodes.push_back(sphereModel);*/

    printTree(cout);
}

bool AddRemoveChildMainScene::onInput(InputEvent& event) {
    bool consumed = false;
    if (Input::isKeyJustPressed(KEY_ENTER)) {
        auto newNode = (randomi(1) == 0) ? crateModel->duplicate() : sphereModel->duplicate();
        newNode->setPosition({randomf(10.0f) - 5, randomf(10.0f) - 5, -10.0f});
        if (addChild(newNode)) rotatingNodes.push_back(newNode);
        consumed = true;
    }
    if (Input::isKeyJustPressed(KEY_BACKSPACE)) {
        if (removeChild(rotatingNodes.back())) rotatingNodes.pop_back();
        consumed = true;
    }
    if (Input::isKeyJustPressed(KEY_SPACE)) {
        if (camera1->isActive()) {
            Application::get().activateCamera(camera2);
        } else {
            Application::get().activateCamera(camera1);
        }
        consumed = true;
    }
    return consumed;
}

void AddRemoveChildMainScene::onPhysicsProcess(float delta) {
    auto angle = delta * radians(90.0f) / 2;
    for (auto& crate: rotatingNodes) {
        crate->rotateY(angle);
        crate->rotateX(angle);
    }
}