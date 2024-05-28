#include <z0/application.h>
#include <z0/gui/gbutton.h>
#include "topbar.h"

TopBar::TopBar(): GWindow(Rect{0, 945, 1000, 55}) {}

void TopBar::onReady() {
    auto rightPadding = make_shared<GPanel>();
    rightPadding->setDrawBackground(false);
    getWidget().add(rightPadding, GWidget::RIGHT, "5,5");
    getWidget().setDrawBackground(false);
    textFPS = make_shared<GText>("9999");
    getWidget().add(textFPS, GWidget::RIGHTCENTER);
    textFPS->setTextColor(Color{1.0, 1.0, 0.2});
    auto buttonQuit = make_shared<GButton>();
    buttonQuit->connect(GEvent::OnClick, this, GEventFunction(&TopBar::onQuit));
    getWidget().add(buttonQuit, GWidget::LEFTCENTER, "10,10", 5);
    auto textQuit = make_shared<GText>("Quit");
    buttonQuit->add(textQuit, GWidget::CENTER);
    buttonQuit->setSize(textQuit->getWidth() + 10, textQuit->getHeight() + 10);
    setHeight(buttonQuit->getHeight());
    setY(1000 - getHeight());
}

void TopBar::onQuit(GWidget &, GEvent *) {
    Application::get().quit();
}

void TopBar::onProcess(float alpha) {
    auto newFPS = Application::get().getFPS();
    if (newFPS != fps) {
        fps = newFPS;
        textFPS->setText(to_string(fps));
    }
}