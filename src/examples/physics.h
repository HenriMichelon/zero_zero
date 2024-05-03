#include <z0/nodes/mesh_instance.h>
#include <z0/nodes/camera.h>
#include <z0/nodes/rigid_body.h>
using namespace z0;

class Player: public Node {
public:
    const float translationSpeed = 4;
    const float mouseSensitivity = 0.002;
    const float viewSensitivity = 0.1;
    const float maxCameraAngleUp = radians(60.0);
    const float maxCameraAngleDown = -radians(45.0);
    Player(): Node("Player") {}

    bool onInput(InputEvent& event) override;
    void onPhysicsProcess(float delta) override;
    void onProcess(float alpha) override;
    void onReady() override;

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
    std::shared_ptr<Camera> camera;

    void captureMouse();
    void releaseMouse();
};

class Crate: public RigidBody {
public:
    explicit Crate(shared_ptr<Node> model);
    void onReady() override;
};

class PhysicsMainScene: public Node {
public:
    PhysicsMainScene(): Node{"Main Scene"} {};
    void onReady() override;
};
