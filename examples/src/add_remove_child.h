using namespace z0;

class AddRemoveChildMainScene: public Node {
public:
    AddRemoveChildMainScene(): Node{"Main Scene"} {};
    void onReady() override;
    bool onInput(InputEvent& event) override;
    void onPhysicsProcess(float delta) override;
private:
    shared_ptr<Node> crateModel;
    shared_ptr<Node> sphereModel;
    list<shared_ptr<Node>> rotatingNodes;
    shared_ptr<Camera> camera1;
    shared_ptr<Camera> camera2;
};
