#include <z0/nodes/mesh_instance.h>
using namespace z0;

class Triangle: public Node {
public:
    Triangle(): Node{"Multicolor triangle"} {};
    void onReady() override;
    void onPhysicsProcess(float delta) override;
private:
    shared_ptr<MeshInstance> triangle;
};

class TriangleMainScene: public Node {
public:
    TriangleMainScene(): Node{"Main Scene"} {};
    void onReady() override;
};
