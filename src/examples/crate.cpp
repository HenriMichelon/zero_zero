#include "crate.h"
#include <z0/application.h>
#include <z0/input.h>
#include <z0/loader.h>

void MainScene::onReady() {
    auto camera = make_shared<Camera>();
    camera->setPosition({0.0f, 0.0f, 1.0f});
    addChild(camera);
    crate1 = Loader::loadModelFromFile("models/crate.glb");
    crate1->setPosition({-5.0f, 0.0f, -10.0f});
    addChild(crate1);

    printTree(cout);
}

void MainScene::onProcess(float alpha) {
    if (Input::isKeyJustPressed(KEY_ENTER)) {
        cout << "ENTER just pressed" << endl;
    }
}

void MainScene::onPhysicsProcess(float delta) {
    auto angle = delta * radians(90.0f) / 2;
    crate1->rotateY(angle);
    crate1->rotateX(angle);
}