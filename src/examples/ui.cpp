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
    setTransparency(0.8f);
    getWidget().setPadding(5);
    getWidget().setTransparency(0.1);
    //getWidget().setDrawBackground(false);
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

    topBar = make_shared<GWindow>(Rect{0, 945, 1000, 55});
    Application::add(topBar);

    auto rightPadding = make_shared<GPanel>();
    rightPadding->setDrawBackground(false);
    topBar->getWidget().add(rightPadding, GWidget::RIGHT, "5,5");
    topBar->getWidget().setDrawBackground(false);

    textFPS = make_shared<GText>("9999");
    topBar->getWidget().add(textFPS, GWidget::RIGHTCENTER);
    textFPS->setTextColor(Color{1.0, 1.0, 0.2});

    auto buttonQuit = make_shared<GButton>();
    buttonQuit->connect(GEvent::OnClick, this, GEventFunction(&UIMainScene::onQuit));
    topBar->getWidget().add(buttonQuit, GWidget::LEFTCENTER, "10,10", 5);

    auto textQuit = make_shared<GText>("Quit");
    buttonQuit->add(textQuit, GWidget::CENTER);
    buttonQuit->setSize(textQuit->getWidth() + 10, textQuit->getHeight() + 10);

    topBar->setHeight(buttonQuit->getHeight());
    topBar->setY(1000 - topBar->getHeight());

    window2 = make_shared<Window2>(Rect{250, 250, 500, 500});
    Application::add(window2);
}

void UIMainScene::onPhysicsProcess(float delta) {
    auto angle = delta * radians(90.0f) / 2;
    sphere->rotateY(angle);
}

void UIMainScene::onProcess(float alpha) {
    auto newFPS = Application::get().getFPS();
    if (newFPS != fps) {
        fps = newFPS;
        textFPS->setText(to_string(fps));
    }
}

void UIMainScene::onQuit(GWidget &, GEvent *) {
    Application::get().quit();
}