#include "z0/z0.h"
using namespace z0;

#include "physics.h"
#include "player.h"
#include "layers.h"

Crate::Crate(const shared_ptr<Node>& model):
    RigidBody{
        make_shared<BoxShape>(vec3{2.0f,2.0f, 2.0f}),
        Layers::BODIES,
        Layers::WORLD | Layers::BODIES | Layers::PLAYER,
        "Crate" } {
    addChild(model);
    setBounce(0.8);
    setGravityScale(0.5);
}

void Crate::onReady() {
    glm::quat rot = angleAxis(radians(static_cast<float>(rand()%90)), AXIS_Z);
    setRotation(rot);
}

void PhysicsMainScene::onReady() {
    auto skybox = make_shared<Skybox>("examples/textures/sky", ".jpg");
    addChild(skybox);
    auto player = make_shared<Player>();
    addChild(player);
    player->setPosition({0.0, 2.0, 2.0});

    auto crateModel = Loader::loadModelFromFile("examples/models/crate.glb", true);
    for (int x = 0; x < 10; x++) {
        for (int z = 0; z < 10; z++) {
            auto model = make_shared<Crate>(crateModel->duplicate());
            model->setPosition({x * 3 - 3 * 5, 3.0 + rand() % 5, -z * 3 - 5});
            addChild(model);
        }
    }

    auto floor = make_shared<StaticBody>(
            make_shared<BoxShape>(vec3{200.0f, 0.2f, 200.0f}),
            Layers::WORLD,
            0,
            "Floor");
    floor->addChild(Loader::loadModelFromFile("examples/models/floor.glb", true));
    floor->setPosition({0.0, -2.0, 0.0});
    addChild(floor);
}

