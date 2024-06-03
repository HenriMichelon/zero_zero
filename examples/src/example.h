#pragma once

class ExampleMainScene: public Node {
public:
    void onReady() override {
        Application::addWindow(make_shared<Menu>());
    }
};
