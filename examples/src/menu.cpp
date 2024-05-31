#include <z0/z0.h>
using namespace z0;

#include "menu.h"
Menu::Menu(): GWindow{Rect{250, 250}}{
}

void Menu::onCreate() {
    cout << "menu" << endl;
}