#pragma once

class ExampleMainScene: public Node, public GEventHandler {
public:
    void onReady() override;
    void onProcess(float alpha) override;

private:
    shared_ptr<GWindow> menu;
    shared_ptr<TopBar> topbar;
    shared_ptr<Node> scene;

    void onMenu(GWidget&, GEvent*);
    void onMenuQuit(GWidget&, GEvent*);
    void onMenuTriangle(GWidget&, GEvent*);
    void onMenuAddRemoveChild(GWidget&, GEvent*);
    void onMenuPhysics(GWidget&, GEvent*);
};
