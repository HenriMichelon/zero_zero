#include <z0/nodes/camera.h>
#include "z0/nodes/skybox.h"
#include "z0/loader.h"
#include "z0/application.h"
#include "z0/gui/gwidget.h"
#include "z0/gui/gbutton.h"
#include "z0/gui/gtoggle_button.h"

#include "ui.h"

#include "stb_image_write.h"

void Window2::onCreate() {
    setTransparency(0.8f);
    getWidget().setPadding(5);

    auto button2 = make_shared<GToggleButton>();
    getWidget().add(button2, GWidget::TOPCENTER, "70,40");

    auto button = make_shared<GButton>();
    button->connect(GEvent::OnClick, this, GEventFunction(&Window2::onButtonClic));
    getWidget().add(button, GWidget::CENTER, "70,40");
}

void Window2::onButtonClic(GWidget &, GEvent *) {
    auto font = Font::create("../examples/LeagueSpartan-Bold.otf", 30);
    uint32_t width, height;
    auto bitmap = font->render("apU", width, height);
    if (stbi_write_png("output.png", width, height, 1, bitmap.data(), width * 1)) {
        std::cout << "Image saved successfully.\n";
    } else {
        std::cerr << "Failed to save the image.\n";
    }
    cout << "BUTTON CLIC" << endl;
}
/*
bool Window2::onKeyDown(Key key) {
    cout << "onKeyDown " << key << endl;
    return false;
};

bool Window2::onKeyUp(Key key) {
    cout << "onKeyUp " << key << endl;
    return false;
};

bool Window2::onMouseDown(MouseButton, uint32_t x, uint32_t y) {
    cout << "onMouseDown " << x << " " << y << endl;
    return true;
};
bool Window2::onMouseUp(MouseButton, uint32_t x, uint32_t y) {
    cout << "onMouseUp " << x << " " << y << endl;
    return true;
};
bool Window2::onMouseMove(MouseButton, uint32_t x, uint32_t y) {
    cout << "onMouseMove " << x << " " << y << endl;
    return true;
};

void Window2::onBoxCreate(GWidget &widget, GEvent*) {
    cout << "BOX CREATE" << endl;
}

void Window2::onBoxMouseDown(z0::GWidget &widget, GEvent *event) {
    auto* mouseEvent = (GEventMouse*)event;
    cout << "BOX MOUSE DOWN" << mouseEvent->x << "x" << mouseEvent->y << endl;
    mouseEvent->consumed = true;
}*/

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