#pragma once

class Menu: public GWindow {
public:
    static Menu* menu;
    Menu();
    void onCreate() override;
private:
    void onMenuQuit(GWidget&, GEvent*);
    void onMenuTriangle(GWidget&, GEvent*);
    void onMenuAddRemoveChild(GWidget&, GEvent*);
};