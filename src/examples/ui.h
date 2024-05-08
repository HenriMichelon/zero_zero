#include <z0/nodes/node.h>
using namespace z0;

class UIMainScene: public Node {
public:
    UIMainScene(): Node{"Main Scene"} {};
    void onReady() override;
    void onPhysicsProcess(float delta) override;
private:
    shared_ptr<Node> sphere;
};
