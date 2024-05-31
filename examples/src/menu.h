#pragma once

class Menu: public GWindow {
public:
    Menu();
    void onCreate() override;
private:
    void onMenuQuit(GWidget&, GEvent*);
    void onMenuTriangle(GWidget&, GEvent*);
    void onMenuAddRemoveChild(GWidget&, GEvent*);
};