#include <z0/nodes/mesh_instance.h>
#include <z0/nodes/camera.h>
using namespace z0;

class AddRemoveChildMainScene: public Node {
public:
    AddRemoveChildMainScene(): Node{"Main Scene"} {};
    void onReady() override;
    void onProcess(float alpha) override;
    void onPhysicsProcess(float delta) override;
private:
    shared_ptr<Node> crateModel;
    list<shared_ptr<Node>> crates;
    shared_ptr<Camera> camera1;
    shared_ptr<Camera> camera2;
};
