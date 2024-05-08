#include <z0/application.h>
#include "triangle.h"
#include "add_remove_child.h"
#include "physics.h"
#include "ui.h"

using namespace z0;

const ApplicationConfig applicationConfig {
    .appName = "Example App",
    .appDir = "..",
    .windowMode = WINDOW_MODE_WINDOWED,
    .windowWidth = 1024,
    .windowHeight = 768,
};
Application application(applicationConfig,
                        make_shared<UIMainScene>());