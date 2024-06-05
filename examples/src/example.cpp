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
        setSize(500, textLabel->getHeight() + 20);
    }
};

void ExampleMainScene::onReady() {
    addChild(make_shared<Camera>());
    auto skybox = make_shared<Skybox>("examples/textures/sky", ".jpg");
    addChild(skybox);

    menu = make_shared<GWindow>(Rect{250, 0, 500, 1000});
    app().addWindow(menu);

    menu->getWidget().setDrawBackground(false);
    menu->getWidget().setPadding(10);
    float height = menu->getWidget().getPadding() * 8;

    auto entryTriangle = make_shared<GMenuEntry>("Triangles & shaders");
    entryTriangle->connect(GEvent::OnClick, this, GEventFunction(&ExampleMainScene::onMenuTriangle));
    menu->getWidget().add(entryTriangle, GWidget::TOPCENTER);
    height += entryTriangle->getHeight();

    auto entryAddRemoveChild = make_shared<GMenuEntry>("Add & remove child");
    entryAddRemoveChild->connect(GEvent::OnClick, this, GEventFunction(&ExampleMainScene::onMenuAddRemoveChild));
    menu->getWidget().add(entryAddRemoveChild, GWidget::TOPCENTER);
    height += entryAddRemoveChild->getHeight();

    auto entryPhysics = make_shared<GMenuEntry>("Physics");
    entryPhysics->connect(GEvent::OnClick, this, GEventFunction(&ExampleMainScene::onMenuPhysics));
    menu->getWidget().add(entryPhysics, GWidget::TOPCENTER);
    height += entryPhysics->getHeight();

    auto entryQuit = make_shared<GMenuEntry>("Quit");
    entryQuit->connect(GEvent::OnClick, this, GEventFunction(&ExampleMainScene::onMenuQuit));
    menu->getWidget().add(entryQuit, GWidget::TOPCENTER);
    height += entryQuit->getHeight();

    menu->setHeight(height);
    menu->setY((1000 - height)/2);

    topbar = make_shared<TopBar>(this, GEventFunction(&ExampleMainScene::onMenu));
    app().addWindow(topbar);

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
    app().quit();
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
    .defaultFontSize = 30
};

Application application(applicationConfig,make_shared<ExampleMainScene>());