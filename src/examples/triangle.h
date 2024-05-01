#include <z0/nodes/mesh_instance.h>
using namespace z0;

class Main: public Node {
public:
    Main(): Node{"Main"} {};
    void onReady() override;
    void onPhysicsProcess(float delta) override;
private:
    shared_ptr<MeshInstance> triangle;
};
