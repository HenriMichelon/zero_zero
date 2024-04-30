#pragma once

#include "z0/constants.h"
#include <filesystem>

namespace z0 {

    struct ApplicationConfig {
        string appName             = "MyApp";
        filesystem::path appDir    = ".";
        WindowMode windowMode      = WINDOW_MODE_WINDOWED;
        uint32_t windowWidth       = 800;
        uint32_t windowHeight      = 600;
        MSAA msaa                  = MSAA_AUTO;
        float gamma                = 1.0f;
        float exposure             = 1.0f;
    };
}