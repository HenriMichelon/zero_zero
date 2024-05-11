#include <z0/nodes/node.h>
using namespace z0;

class Window2 : public GWindow {
public:
    explicit Window2(Rect rect) : GWindow{rect} {}

    void onCreate() override;
    void onDestroy() override {
        cout << "onDestroy" << endl;
    };
    void onResize() override {
        cout << "onResize" << endl;
    };
    void onMove() override {
        cout << "onMove" << endl;
    };
    void onShow() override {
        cout << "onShow" << endl;
    };
    void onHide() override {
        cout << "onHide" << endl;
    };
    bool onKeyDown(Key key) override {
        cout << "onKeyDown " << key << endl;
        return false;
    };
    bool onKeyUp(Key key) override {
        cout << "onKeyUp " << key << endl;
        return false;
    };
    void onMouseDown(MouseButton, int32_t, int32_t) override {
        cout << "onMouseDown" << endl;
    };
    void onMouseUp(MouseButton, int32_t, int32_t) override {
        cout << "onMouseUp" << endl;
    };
    void onMouseMove(MouseButton, int32_t, int32_t) override {
        cout << "onMouseMove" << endl;
    };
    void onGotFocus() override {
        cout << "onGotFocus" << endl;
    };
    void onLostFocus() override {
        cout << "onLostFocus" << endl;
    };
};

class UIMainScene: public Node {
public:
    UIMainScene(): Node{"Main Scene"} {};
    void onReady() override;
    void onPhysicsProcess(float delta) override;
    void onProcess(float alpha) override;
private:
    shared_ptr<Node> sphere;
    shared_ptr<Window2> window2;
};
