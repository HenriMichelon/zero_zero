#include "crate.h"
#include <z0/application.h>
#include <z0/input.h>
#include <z0/loader.h>

void MainScene::onReady() {
    auto camera = make_shared<Camera>();
    camera->setPosition({0.0f, 0.0f, 1.0f});
    addChild(camera);
    crateModel = Loader::loadModelFromFile("models/crate.glb");
    crateModel->setPosition({-5.0f, 0.0f, -10.0f});
    addChild(crateModel);
    crates.push_back(crateModel);

    printTree(cout);
}

void MainScene::onProcess(float alpha) {
    if (Input::isKeyJustPressed(KEY_ENTER)) {
        auto crate2 = crateModel->duplicate();
        crate2->setPosition({rand() % 10 - 5, rand() % 10 - 5, -10.0f});
        addChild(crate2);
        crates.push_back(crate2);
    }
}

void MainScene::onPhysicsProcess(float delta) {
    auto angle = delta * radians(90.0f) / 2;
    for (auto& crate: crates) {
        crate->rotateY(angle);
        crate->rotateX(angle);
    }
}