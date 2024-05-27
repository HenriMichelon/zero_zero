#include <z0/nodes/node.h>
#include <z0/gui/gevent.h>
#include <z0/gui/gtext.h>
using namespace z0;

class Window2 : public GWindow {
public:
    explicit Window2(Rect rect) : GWindow{rect} {}

    void onCreate() override;

private:
    void onButtonClic(GWidget&, GEvent*);
};

class UIMainScene: public Node {
public:
    UIMainScene(): Node{"Main Scene"} {};
    void onReady() override;
    void onPhysicsProcess(float delta) override;
    void onProcess(float alpha) override;
    bool onInput(z0::InputEvent &inputEvent) override;
private:
    uint32_t fps{0};
    shared_ptr<Node> sphere;
    shared_ptr<GWindow> topBar;
    shared_ptr<Window2> window2;
    shared_ptr<GText> textFPS;

    void onQuit(GWidget&, GEvent*);
};
