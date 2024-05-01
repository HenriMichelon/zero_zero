#include <z0/nodes/mesh_instance.h>
using namespace z0;

class MainScene: public Node {
public:
    MainScene(): Node{"Main Scene"} {};
    void onReady() override;
    void onProcess(float alpha) override;
    void onPhysicsProcess(float delta) override;
private:
    shared_ptr<Node> crate1;
};
