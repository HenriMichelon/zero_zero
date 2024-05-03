#include <z0/nodes/node.h>
using namespace z0;

class UIMainScene: public Node {
public:
    UIMainScene(): Node{"Main Scene"} {};
    void onReady() override;
    void onProcess(float alpha) override;
};
