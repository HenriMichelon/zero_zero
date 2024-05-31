#include <z0/z0.h>
using namespace z0;

#include "menu.h"
#include "triangle.h"
#include "add_remove_child.h"
#include "ui.h"
#include "physics.h"

class GMenuEntry: public GButton {
public:
    explicit GMenuEntry(const string& _label): GButton{}, label{_label} {
        this->connect(GEvent::OnCreate, this, GEventFunction(&GMenuEntry::onCreate));
    }

private:
    const string label;
    void onCreate(GWidget&, GEvent*) {
        auto textLabel = make_shared<GText>(label);
        add(textLabel, CENTER);
        setPadding(10);
        setSize(500, textLabel->getHeight() + getPadding()*2);
    }
};

Menu::Menu(): GWindow{Rect{250, 250, 500, 500}}{
}

void Menu::onMenuQuit(z0::GWidget &, z0::GEvent *) {
    Application::quit();
}

void Menu::onMenuTriangle(z0::GWidget &, z0::GEvent *) {
    //hide();
    Application::get().setRootNode(make_shared<TriangleMainScene>());
}

void Menu::onMenuAddRemoveChild(z0::GWidget &, z0::GEvent *) {
    //hide();
    Application::get().setRootNode(make_shared<AddRemoveChildMainScene>());
}

void Menu::onCreate() {
    getWidget().setDrawBackground(false);
    getWidget().setPadding(10);

    auto entryTriangle = make_shared<GMenuEntry>("Triangles & shaders");
    entryTriangle->connect(z0::GEvent::OnClick, this, GEventFunction(&Menu::onMenuTriangle));
    getWidget().add(entryTriangle, GWidget::TOPCENTER);

    auto entryAddRemoveChild = make_shared<GMenuEntry>("Add & remove child");
    entryAddRemoveChild->connect(z0::GEvent::OnClick, this, GEventFunction(&Menu::onMenuAddRemoveChild));
    getWidget().add(entryAddRemoveChild, GWidget::TOPCENTER);

    auto entryQuit = make_shared<GMenuEntry>("Quit");
    entryQuit->connect(z0::GEvent::OnClick, this, GEventFunction(&Menu::onMenuQuit));
    getWidget().add(entryQuit, GWidget::TOPCENTER);
}