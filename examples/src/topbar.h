#pragma once

class TopBar: public GWindow {
public:
    TopBar(GEventHandler* handler, GEventFunction onQuit);
    void onCreate() override;
    void updateFPS();

private:
    uint32_t fps{0};
    shared_ptr<GText> textFPS;
    GEventHandler* onQuitHandler;
    GEventFunction onQuit;
};