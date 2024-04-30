#include <z0/application.h>

using namespace z0;

class Main: public Node {
public:
    Main(): Node{"Main"} {};

    void onReady() override {
        cout << *this << ".onReady" << endl;
        cout << Application::get().getWindow() << endl;
    }
};

const ApplicationConfig applicationConfig {
    .appName = "Example App",
    .appDir = "..",
    .windowMode = z0::WINDOW_MODE_WINDOWED,
    .windowWidth = 1024,
    .windowHeight = 768,
};
Application application(applicationConfig, make_shared<Main>());