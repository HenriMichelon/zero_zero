#include "physics.h"
#include "z0/nodes/skybox.h"
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
    auto skybox = make_shared<Skybox>("examples/textures/sky", ".jpg");
    addChild(skybox);
    addChild(make_shared<Player>());

    auto crateModel= z0::Loader::loadModelFromFile("examples/models/crate.glb", true);
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
    floor->addChild(Loader::loadModelFromFile("examples/models/floor.glb", true));
    floor->setPosition({0.0, -2.0, 0.0});
    addChild(floor);

}

bool Player::onInput(InputEvent& event) {
    /*if ((event.getType() == z0::INPUT_EVENT_MOUSE_MOTION) && mouseCaptured) {
        auto& eventMouseMotion = dynamic_cast<z0::InputEventMouseMotion&>(event);
        rotateY(-eventMouseMotion.getRelativeX() * mouseSensitivity);
        camera->rotateX(eventMouseMotion.getRelativeY() * mouseSensitivity * mouseInvertedAxisY);
        camera->setRotationX(std::clamp(camera->getRotationX(), maxCameraAngleDown, maxCameraAngleUp));
    }*/
    if ((event.getType() == INPUT_EVENT_KEY) && mouseCaptured) {
        auto& eventKey = dynamic_cast<InputEventKey&>(event);
        if ((eventKey.getKeyCode() == KEY_ESCAPE) && !eventKey.isPressed()) {
            releaseMouse();
            return true;
        }
    }
    return false;
}

void Player::onPhysicsProcess(float delta) {
    /*previousState = currentState;
    glm::vec2 input;
    if (gamepad != -1) {
        input = z0::Input::getGamepadVector(gamepad, z0::GAMEPAD_AXIS_LEFT);
        if (input == z0::VEC2ZERO) input = z0::Input::getKeyboardVector(z0::KEY_A, z0::KEY_D, z0::KEY_W, z0::KEY_S);
    } else {
        input = z0::Input::getKeyboardVector(z0::KEY_A, z0::KEY_D, z0::KEY_W, z0::KEY_S);
    }

    currentState = State{};
    if (input != z0::VEC2ZERO) {
        auto direction = transformBasis * glm::vec3{input.x, 0, input.y};
        currentState.velocity.x = direction.x * translationSpeed;
        currentState.velocity.z = direction.z * translationSpeed;
    }
    if (z0::Input::isKeyPressed(z0::KEY_Q)) {
        currentState.velocity.y += translationSpeed / 2;
    } else if (z0::Input::isKeyPressed(z0::KEY_Z)) {
        currentState.velocity.y -= translationSpeed / 2;
    }
    if (currentState.velocity != z0::VEC3ZERO) currentState.velocity *= delta;

    if (mouseCaptured) {
        glm::vec2 inputDir;
        if (gamepad != -1) {
            inputDir = z0::Input::getGamepadVector(gamepad, z0::GAMEPAD_AXIS_RIGHT);
            if (inputDir == z0::VEC2ZERO) inputDir = z0::Input::getKeyboardVector(z0::KEY_LEFT, z0::KEY_RIGHT, z0::KEY_UP, z0::KEY_DOWN);
        } else {
            inputDir = z0::Input::getKeyboardVector(z0::KEY_LEFT, z0::KEY_RIGHT, z0::KEY_UP, z0::KEY_DOWN);
        }
        if (inputDir != z0::VEC2ZERO) currentState.lookDir = inputDir * viewSensitivity * delta;
    }*/
}

void Player::onProcess(float alpha) {
    /*if (currentState.velocity != z0::VEC3ZERO) {
        captureMouse();
        auto interpolatedVelocity = previousState.velocity * (1.0f-alpha) + currentState.velocity * alpha;
        translate(interpolatedVelocity);
    }
    if (currentState.lookDir != z0::VEC2ZERO) {
        captureMouse();
        auto interpolatedLookDir = previousState.lookDir * (1.0f-alpha) + currentState.lookDir * alpha;
        rotateY(-interpolatedLookDir.x * 2.0f);
        camera->rotateX(interpolatedLookDir.y * keyboardInvertedAxisY);
        camera->setRotationX(std::clamp(camera->getRotationX() , maxCameraAngleDown, maxCameraAngleUp));
    }*/
}

void Player::onReady() {
    captureMouse();
    setPosition({0.0, 1.5, 10.0});

    camera = std::make_shared<z0::Camera>();
    addChild(camera);
/*
    for (int i = 0; i < z0::Input::getConnectedJoypads(); i++) {
        if (z0::Input::isGamepad(i)) {
            gamepad = i;
            break;
        }
    }
    if (gamepad != -1) {
        std::cout << "Using Gamepad " << z0::Input::getGamepadName(gamepad) << std::endl;
    }*/
}

void Player::captureMouse() {
    Input::setMouseMode(MOUSE_MODE_HIDDEN_CAPTURED);
    mouseCaptured = true;
}

void Player::releaseMouse() {
    Input::setMouseMode(MOUSE_MODE_VISIBLE);
    mouseCaptured = false;
}

//TODO : key code + translate, mouse inputevent, mouse buttons,