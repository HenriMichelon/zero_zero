#include "example.h"
#include "menu.h"

const ApplicationConfig applicationConfig {
    .appName = "Example App",
    .appDir = "..",
    .windowMode = WINDOW_MODE_WINDOWED,
    .windowWidth = 1024,
    .windowHeight = 768,
    .defaultFontName = "examples/Signwood.ttf",
    .defaultFontSize = 16
};

class ExampleMainScene: public Node {
public:
    void onReady() override {
        Application::addWindow(make_shared<Menu>());
    }
};

Application application(applicationConfig,make_shared<ExampleMainScene>());