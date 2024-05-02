#include "physics.h"
#include <z0/application.h>
#include <z0/input.h>
#include <z0/loader.h>
#include <z0/nodes/static_body.h>

#include <glm/gtc/quaternion.hpp>

enum Layers {
    WORLD       = 0b0001,
    BODIES      = 0b0010,
};

Crate::Crate(shared_ptr<z0::Node> model):
    RigidBody{make_shared<z0::BoxShape>(vec3{2.0f,2.0f, 2.0f}),
    Layers::BODIES,
    Layers::WORLD | Layers::BODIES} {
    addChild(std::move(model));
    setBounce(0.8);
    setGravityScale(0.5);
}

void Crate::onReady() {
    glm::quat rot = angleAxis(radians(static_cast<float>(rand()%90)), AXIS_Z);
    setRotation(rot);
}

void PhysicsMainScene::onReady() {
    auto camera= make_shared<Camera>("Camera 1");
    camera->setPosition({0.0f, 0.0f, 10.0f});
    addChild(camera);

    auto crateModel= z0::Loader::loadModelFromFile("models/crate.glb", true);
    for (int x = 0; x < 10; x++) {
        for (int z = 0; z < 10; z++) {
            auto model= make_shared<Crate>(crateModel->duplicate());
            model->setPosition({x * 3 - 3*5, 3.0 + rand() % 5, -z * 3 - 5});
            addChild(model);
        }
    }

    auto floor= make_shared<StaticBody>(
            make_shared<BoxShape>(vec3{200.0f,0.2f, 200.0f}),
            Layers::WORLD,
            0);
    floor->addChild(Loader::loadModelFromFile("models/floor.glb", true));
    floor->setPosition({0.0, -2.0, 0.0});
    addChild(floor);

}
