#include "z0/z0.h"
using namespace z0;

#include "topbar.h"
#include "menu.h"

const ApplicationConfig applicationConfig {
    .appName = "Example App",
    .appDir = "..",
    .windowMode = WINDOW_MODE_WINDOWED,
    .windowWidth = 800,
    .windowHeight = 600,
    .defaultFontName = "examples/Signwood.ttf",
    .defaultFontSize = 12
};

class ExampleMainScene: public Node {
public:
    void onReady() override {
        auto topbar = make_shared<TopBar>();
        Application::addWindow(topbar);
        addChild(topbar);

        auto menu = make_shared<Menu>();
        Application::addWindow(menu);
    }
};

Application application(applicationConfig,make_shared<ExampleMainScene>());