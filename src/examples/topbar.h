#pragma once

class TopBar: public GWindow, public Node {
public:
    TopBar();
    void onReady() override;
    void onProcess(float alpha) override;
private:
    uint32_t fps{0};
    shared_ptr<GText> textFPS;
    void onQuit(GWidget&, GEvent*);
};