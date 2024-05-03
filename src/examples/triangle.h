#include <z0/nodes/mesh_instance.h>
using namespace z0;

class Triangle: public Node {
public:
    Triangle(): Node{"Multicolor triangle"} {};
    void onReady() override;
    void onPhysicsProcess(float delta) override;
    void onProcess(float alpha) override;
    bool onInput(InputEvent& inputEvent) override;

private:
    shared_ptr<MeshInstance> triangle1;
    shared_ptr<MeshInstance> triangle2;
    shared_ptr<StandardMaterial> material1;
    shared_ptr<ShaderMaterial> material2;
    float gradientSpeed{0.5f};
    float gradient{0.0f};
    bool rotate{true};
};

class TriangleMainScene: public Node {
public:
    TriangleMainScene(): Node{"Main Scene"} {};
    void onReady() override;
};
