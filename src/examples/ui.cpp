#include <z0/input.h>
#include <z0/nodes/camera.h>
#include "z0/nodes/skybox.h"
#include "z0/loader.h"
#include "z0/application.h"
#include "z0/gui/gwidget.h"

#include "ui.h"

void Window2::onCreate() {
    setTransparency(0.5f);
    //getLayout()->setOption("color_background", "1.0,0.647,0.0,0.5");
    auto box = make_shared<GWidget>(GWidget::Type::BOX);
    box->connect(GEvent::OnCreate, this, GEventFunction(&Window2::onBoxCreate));
    getWidget().add(box, GWidget::CENTER, "70,40,RAISED");
}

bool Window2::onKeyDown(Key key) {
    cout << "onKeyDown " << key << endl;
    return false;
};

bool Window2::onKeyUp(Key key) {
    cout << "onKeyUp " << key << endl;
    return false;
};

void Window2::onBoxCreate(GWidget &widget, GEvent*) {
    cout << "BOX CREATE" << endl;
}

bool UIMainScene::onInput(InputEvent &inputEvent) {
    cout << "UIMainScene input" << endl;
    if (inputEvent.getType() == INPUT_EVENT_KEY) {
        auto& keyInputEvent = dynamic_cast<InputEventKey&>(inputEvent);
        if ((keyInputEvent.getKeyCode() == KEY_ENTER) && keyInputEvent.isPressed()) {
            window2->setVisible(!window2->isVisible());
            return true;
        }
    }
    return false;
}

void UIMainScene::onReady() {
    auto camera = make_shared<Camera>();
    addChild(camera);
    auto skybox = make_shared<Skybox>("examples/textures/sky", ".jpg");
    addChild(skybox);
    sphere = Loader::loadModelFromFile("examples/models/sphere.glb");
    sphere->setPosition({0.0f, 0.0f, -5.0f});
    addChild(sphere);
    Application::add(make_shared<GWindow>(Rect{250, 950, 500, 25}));
    window2 = make_shared<Window2>(Rect{250, 250, 500, 500});
    Application::add(window2);
}

void UIMainScene::onPhysicsProcess(float delta) {
    auto angle = delta * radians(90.0f) / 2;
    sphere->rotateY(angle);
}

void UIMainScene::onProcess(float alpha) {

}