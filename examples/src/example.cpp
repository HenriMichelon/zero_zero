#include "includes.h"
#include "menu.h"
#include "example.h"

const ApplicationConfig applicationConfig {
    .appName = "Example App",
    .appDir = "..",
    .windowMode = WINDOW_MODE_WINDOWED,
    .windowWidth = 800,
    .windowHeight = 600,
    .defaultFontName = "examples/Signwood.ttf",
    .defaultFontSize = 16
};

Application application(applicationConfig,make_shared<ExampleMainScene>());