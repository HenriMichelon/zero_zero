#include <z0/nodes/node.h>
using namespace z0;

class Window2 : public GWindow {
public:
    explicit Window2(GRect rect) : GWindow{rect} {}

    void onCreate() override {
        cout << "onCreate" << endl;
    };
    void onDestroy() override {
        cout << "onDestroy" << endl;
    };
    void onShow() override {
        cout << "onShow" << endl;
    };
    void onHide() override {
        cout << "onHide" << endl;
    };
    void onKeybDown(Key) override {
        cout << "onKeybDown" << endl;
    };
    void onKeybUp(Key) override {
        cout << "onKeybUp" << endl;
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
