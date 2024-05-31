#pragma once

class Player: public Character {
public:
    const float movementsSpeed = 4;
    const float jumpHeight = 8;
    const float mouseSensitivity = 0.008;
    const float viewSensitivity = 0.2;
    const float maxCameraAngleUp = radians(60.0);
    const float maxCameraAngleDown = -radians(45.0);
    Player();

    bool onInput(InputEvent& event) override;
    void onPhysicsProcess(float delta) override;
    void onProcess(float alpha) override;
    void onReady() override;
    void onCollisionStarts(PhysicsNode* node) override;

private:
    struct State {
        vec3 velocity = VEC3ZERO;
        vec2 lookDir = VEC2ZERO;
        State& operator=(const State& other) = default;
    };

    int gamepad{-1};
    bool mouseCaptured{false};
    float mouseInvertedAxisY{1.0};
    float keyboardInvertedAxisY{1.0};
    State previousState;
    State currentState;
    shared_ptr<Node> cameraPivot;

    void captureMouse();
    void releaseMouse();

    shared_ptr<Node> model;
};