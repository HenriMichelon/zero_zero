#pragma once

class Crate: public RigidBody {
public:
    explicit Crate(const shared_ptr<Node>& model);
    void onReady() override;
};

class PhysicsMainScene: public Node {
public:
    PhysicsMainScene(): Node{"Main Scene"} {};
    void onReady() override;
};
