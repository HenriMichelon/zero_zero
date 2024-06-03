#pragma once

class TopBar: public GWindow {
public:
    TopBar(GEventFunction onQuit);
    void onCreate() override;
    void updateFPS();

private:
    uint32_t fps{0};
    shared_ptr<GText> textFPS;
    GEventFunction onQuit;
};