#include <z0/nodes/node.h>
using namespace z0;

class Main: public Node {
public:
    Main(): Node{"Main"} {};
    void onReady() override;
};
