#include <z0/z0.h>
using namespace z0;

#include "add_remove_child.h"
#include "triangle.h"
#include "ui.h"
#include "physics.h"

const ApplicationConfig applicationConfig {
    .appName = "Example App",
    .appDir = "..",
    .windowMode = WINDOW_MODE_WINDOWED,
    .windowWidth = 800,
    .windowHeight = 600,
    .defaultFontName = "examples/Signwood.ttf",
    .defaultFontSize = 12
};
Application application(applicationConfig,make_shared<PhysicsMainScene>());