#include <z0/nodes/mesh_instance.h>
#include <z0/nodes/camera.h>
#include <z0/nodes/rigid_body.h>
using namespace z0;

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
