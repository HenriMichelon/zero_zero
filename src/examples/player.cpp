#include <z0/input.h>
#include <z0/loader.h>
#include <algorithm>

#include "player.h"
#include "layers.h"

Player::Player(): RigidBody{make_shared<BoxShape>(vec3{1.0f,2.0f, 1.0f}),
                            Layers::PLAYER,
                            Layers::WORLD | Layers::BODIES} {
}

bool Player::onInput(InputEvent& event) {
    if ((event.getType() == INPUT_EVENT_MOUSE_MOTION) && mouseCaptured) {
        auto& eventMouseMotion = dynamic_cast<InputEventMouseMotion&>(event);
        //rotateY(-eventMouseMotion.getRelativeX() * mouseSensitivity);
        camera->rotateX(eventMouseMotion.getRelativeY() * mouseSensitivity * mouseInvertedAxisY);
        camera->setRotationX(std::clamp(camera->getRotationX(), maxCameraAngleDown, maxCameraAngleUp));
        return true;
    }
    if ((event.getType() == INPUT_EVENT_MOUSE_BUTTON) && (!mouseCaptured)) {
        auto& eventMouseButton = dynamic_cast<InputEventMouseButton&>(event);
        if (!eventMouseButton.isPressed()) {
            captureMouse();
            return true;
        }
    }
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
    previousState = currentState;
    vec2 input;
    if (gamepad != -1) {
        //input = Input::getGamepadVector(gamepad, GAMEPAD_AXIS_LEFT);
        if (input == VEC2ZERO) input = Input::getKeyboardVector(KEY_A, KEY_D, KEY_W, KEY_S);
    } else {
        input = Input::getKeyboardVector(KEY_A, KEY_D, KEY_W, KEY_S);
    }

    currentState = State{};
    if (input != VEC2ZERO) {
        captureMouse();
        auto direction = TRANSFORM_BASIS * vec3{input.x, 0, input.y};
        currentState.velocity.x = direction.x * translationSpeed;
        currentState.velocity.z = direction.z * translationSpeed;
    }
    if (Input::isKeyPressed(KEY_Q)) {
        currentState.velocity.y += translationSpeed / 2;
    } else if (Input::isKeyPressed(KEY_Z)) {
        currentState.velocity.y -= translationSpeed / 2;
    }
    if (currentState.velocity != VEC3ZERO) {
        currentState.velocity *= delta;
    }

    if (mouseCaptured) {
        vec2 inputDir;
        if (gamepad != -1) {
            //inputDir = Input::getGamepadVector(gamepad, GAMEPAD_AXIS_RIGHT);
            if (inputDir == VEC2ZERO) inputDir = Input::getKeyboardVector(KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN);
        } else {
            inputDir = Input::getKeyboardVector(KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN);
        }
        if (inputDir != VEC2ZERO) {
            currentState.lookDir = inputDir * viewSensitivity * delta;
        }
    }
}

void Player::onProcess(float alpha) {
    if (currentState.velocity != VEC3ZERO) {
        auto interpolatedVelocity = previousState.velocity * (1.0f-alpha) + currentState.velocity * alpha;
        translate(interpolatedVelocity);
    }
    if (currentState.lookDir != VEC2ZERO) {
        auto interpolatedLookDir = previousState.lookDir * (1.0f-alpha) + currentState.lookDir * alpha;
        rotateY(-interpolatedLookDir.x * 2.0f);
        camera->rotateX(interpolatedLookDir.y * keyboardInvertedAxisY);
        camera->setRotationX(std::clamp(camera->getRotationX() , maxCameraAngleDown, maxCameraAngleUp));
    }
}

void Player::onReady() {
    model = Loader::loadModelFromFile("examples/models/capsule.glb", true);
    model->setScale(0.5);
    addChild(model);

    captureMouse();

    camera = make_shared<Camera>();
    camera->setPosition({0.0, 2.0, 2.0});
    addChild(camera);
/*
    for (int i = 0; i < Input::getConnectedJoypads(); i++) {
        if (Input::isGamepad(i)) {
            gamepad = i;
            break;
        }
    }
    if (gamepad != -1) {
        std::cout << "Using Gamepad " << Input::getGamepadName(gamepad) << std::endl;
    }*/
}

void Player::captureMouse() {
    if (!mouseCaptured) {
        Input::setMouseMode(MOUSE_MODE_HIDDEN_CAPTURED);
        mouseCaptured = true;
    }
}

void Player::releaseMouse() {
    Input::setMouseMode(MOUSE_MODE_VISIBLE);
    mouseCaptured = false;
}