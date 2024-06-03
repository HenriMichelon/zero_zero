#include "includes.h"
#include "topbar.h"
#include "example.h"
#include "triangle.h"
#include "add_remove_child.h"
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

void ExampleMainScene::onReady() {
    menu = make_shared<GWindow>(Rect{250, 250, 500, 500});
    Application::addWindow(menu);

    menu->getWidget().setDrawBackground(false);
    menu->getWidget().setPadding(10);

    auto entryTriangle = make_shared<GMenuEntry>("Triangles & shaders");
    entryTriangle->connect(GEvent::OnClick, this, GEventFunction(&ExampleMainScene::onMenuTriangle));
    menu->getWidget().add(entryTriangle, GWidget::TOPCENTER);

    auto entryAddRemoveChild = make_shared<GMenuEntry>("Add & remove child");
    entryAddRemoveChild->connect(GEvent::OnClick, this, GEventFunction(&ExampleMainScene::onMenuAddRemoveChild));
    menu->getWidget().add(entryAddRemoveChild, GWidget::TOPCENTER);

    auto entryPhysics = make_shared<GMenuEntry>("Physics");
    entryPhysics->connect(GEvent::OnClick, this, GEventFunction(&ExampleMainScene::onMenuPhysics));
    menu->getWidget().add(entryPhysics, GWidget::TOPCENTER);

    auto entryQuit = make_shared<GMenuEntry>("Quit");
    entryQuit->connect(GEvent::OnClick, this, GEventFunction(&ExampleMainScene::onMenuQuit));
    menu->getWidget().add(entryQuit, GWidget::TOPCENTER);

    topbar = make_shared<TopBar>(this, GEventFunction(&ExampleMainScene::onMenu));
    Application::addWindow(topbar);

    scene = make_shared<Node>();
    addChild(scene);
}

void ExampleMainScene::onProcess(float alpha) {
    topbar->updateFPS();
}

void ExampleMainScene::onMenu(GWidget &, GEvent *) {
    scene->removeAllChildren();
    topbar->hide();
    menu->show();
}

void ExampleMainScene::onMenuQuit(GWidget &, GEvent *) {
    Application::quit();
}

void ExampleMainScene::onMenuTriangle(GWidget &, GEvent *) {
    menu->hide();
    topbar->show();
    scene->addChild(make_shared<TriangleMainScene>());
}

void ExampleMainScene::onMenuAddRemoveChild(GWidget &, GEvent *) {
    menu->hide();
    topbar->show();
    scene->addChild(make_shared<AddRemoveChildMainScene>());
}

void ExampleMainScene::onMenuPhysics(GWidget &, GEvent *) {
    menu->hide();
    topbar->show();
    scene->addChild(make_shared<PhysicsMainScene>());
}

const ApplicationConfig applicationConfig {
    .appName = "Example App",
    .appDir = "..",
    .windowMode = WINDOW_MODE_WINDOWED,
    .windowWidth = 800,
    .windowHeight = 600,
    .defaultFontName = "examples/Signwood.ttf",
    .defaultFontSize = 16
};

Application application(applicationConfig,make_shared<ExampleMainScene>());