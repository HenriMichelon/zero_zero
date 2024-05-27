#include <z0/nodes/camera.h>
#include "z0/nodes/skybox.h"
#include "z0/loader.h"
#include "z0/application.h"
#include "z0/gui/gwidget.h"
#include "z0/gui/gbutton.h"
#include "z0/gui/gtoggle_button.h"
#include "z0/gui/gtext.h"
#include "z0/gui/gframe.h"

#include "ui.h"

void Window2::onCreate() {
    //setTransparency(0.8f);
    getWidget().setPadding(5);
/*
    auto frame = make_shared<GFrame>("Hello Frame !");
    getWidget().add(frame, GWidget::CENTER, "200,200", 10);
    frame->add(make_shared<GText>("Text 1"), GWidget::TOPCENTER);
    frame->add(make_shared<GText>("Text 2"), GWidget::BOTTOMCENTER);
*/
    auto button = make_shared<GButton>();
    button->connect(GEvent::OnClick, this, GEventFunction(&Window2::onButtonClic));
    getWidget().add(button, GWidget::CENTER, "70,40", 5);
    button->add(make_shared<GText>("Clic me !"), GWidget::CENTER);
}

void Window2::onButtonClic(GWidget &, GEvent *) {
    cout << "BUTTON CLIC" << endl;
}

bool UIMainScene::onInput(InputEvent &inputEvent) {
    if (inputEvent.getType() == INPUT_EVENT_KEY) {
        auto& keyInputEvent = dynamic_cast<InputEventKey&>(inputEvent);
        if ((keyInputEvent.getKeyCode() == KEY_ENTER) && keyInputEvent.isPressed()) {
            window2->setVisible(!window2->isVisible());
            return true;
        }
    } else if (inputEvent.getType() == INPUT_EVENT_MOUSE_BUTTON) {
        auto &mouseInputEvent = dynamic_cast<InputEventMouseButton&>(inputEvent);
        //cout << mouseInputEvent.getX() << " " << mouseInputEvent.getY() << endl;
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
    //Application::add(make_shared<GWindow>(Rect{250, 950, 500, 25}));
    window2 = make_shared<Window2>(Rect{250, 250, 500, 500});
    Application::add(window2);
}

void UIMainScene::onPhysicsProcess(float delta) {
    auto angle = delta * radians(90.0f) / 2;
    sphere->rotateY(angle);
}

void UIMainScene::onProcess(float alpha) {

}